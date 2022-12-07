#include "B3L/Hook.h"
#include "B3L/LambdaTraits.h"
#include "Windows.h"
#include <Xinput.h>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <string>

using namespace B3L;

// Link XInput as a module that exports functions by ordinal
#pragma comment(lib, "XInput.lib")
namespace {
    auto _0 = &XInputGetState; // This is needed so XInputGetState gets imported into the test binary.
}

class IatHookTests : public ::testing::Test {
protected:
    using GetCurrentProcessId_t     = decltype(&GetCurrentProcessId);
    using GetCurrentProcessIdHook_t = IatHook<GetCurrentProcessId_t>;

    IatHookTests() = default;

    static std::unique_ptr<IHook> createHook();
    static bool hookIsActive();
};

TEST_F(IatHookTests, ConstructionExpections) {
    auto proc = reinterpret_cast<void (*)()>(123);
    // Empty module name
    EXPECT_THROW(IatHook("", "", proc), std::invalid_argument);
    // Empty function name
    EXPECT_THROW(IatHook("abc", "", proc), std::invalid_argument);
    // Invalid ordinal
    EXPECT_THROW(IatHook("abc", -1, proc), std::invalid_argument);
    // Proc is nullptr
    EXPECT_THROW(IatHook("abc", "abc", reinterpret_cast<void (*)()>(0)), std::invalid_argument);
    // Import not found
    EXPECT_THROW(IatHook("abc", 0, proc), std::exception);
}

TEST_F(IatHookTests, ConstructWithOrdinal) {
    using XInputGetState_t     = decltype(&XInputGetState);
    using XInputGetStateHook_t = IatHook<XInputGetState_t>;

    int XInputGetStateOrdinal                 = 2;
    static auto proc                          = [](DWORD, XINPUT_STATE*) { return 0ul; };
    unsigned long (*fp)(DWORD, XINPUT_STATE*) = proc;

    auto h = std::make_unique<XInputGetStateHook_t>("XINPUT1_4.dll", XInputGetStateOrdinal, fp);
}

TEST_F(IatHookTests, ConstructWithName) {
    auto hook = createHook();
}

TEST_F(IatHookTests, MovingInvalidatesSource) {
    auto hook = createHook();
    EXPECT_TRUE(hook->isValid());

    auto hook2 = hook->move();
    EXPECT_FALSE(hook->isValid());
}

// Tests whether moved from hooks skip resource freeing on distruction
TEST_F(IatHookTests, MovedFromCorrectDestruction) {
    auto hook = createHook();
    hook->enable();

    auto hook2 = hook->move();
    hook.reset();

    EXPECT_TRUE(hookIsActive());
}

TEST_F(IatHookTests, InvokeOriginalOnBase) {
    auto pid = GetCurrentProcessId();

    auto hook = createHook();
    EXPECT_TRUE(hook->enable());

    auto pid1 = hook->invokeOriginal<unsigned long(void)>();
    EXPECT_EQ(pid, pid1);
}

TEST_F(IatHookTests, InvokeOriginal) {
    auto pid = GetCurrentProcessId();

    auto hook = dynamic_cast<IatHook<decltype(&GetCurrentProcessId)>*>(createHook().release());
    EXPECT_TRUE(hook->enable());

    auto pid1 = hook->invokeOriginal();
    EXPECT_EQ(pid, pid1);
    delete hook;
}

TEST_F(IatHookTests, ConstructDisabled) {
    auto hook = createHook();
    EXPECT_FALSE(hook->isEnabled());

    EXPECT_FALSE(hookIsActive());
}

TEST_F(IatHookTests, EnableDisable) {
    auto hook = createHook();
    EXPECT_TRUE(hook->enable());
    EXPECT_FALSE(hook->enable());

    EXPECT_TRUE(hookIsActive());

    EXPECT_TRUE(hook->disable());
    EXPECT_FALSE(hook->disable());

    EXPECT_FALSE(hookIsActive());
}

TEST_F(IatHookTests, DisableOnDestruct) {
    {
        auto hook = createHook();
        hook->enable();

        EXPECT_TRUE(hookIsActive());
    }

    EXPECT_FALSE(hookIsActive());
}

// Vft Hook
class VftHookTests : public ::testing::Test {
protected:
    struct Foo {
        virtual int echoInt(int i) const {
            return i;
        }
    };

    static inline Foo foo{};
    static inline int v                        = 123;
    static inline int (*doubleInt)(void*, int) = [](void*, int i) { return 2 * i; };

    static bool isNotHooked(const Foo& foo_) {
        return foo_.echoInt(v) == v;
    }

    static bool isHooked(const Foo& foo_) {
        return foo_.echoInt(v) == doubleInt(nullptr, v);
    }
};

namespace {

    template <typename T>
    struct VTable;

    template <>
    struct VTable<VftHookTests::Foo> {
        int (*echoInt)(void*, int);
    };

} // namespace

// Construct Hooks in disabled state
TEST_F(VftHookTests, ConstructDisabled) {
    VftHook hook{ &foo, &VTable<Foo>::echoInt, doubleInt };
    EXPECT_FALSE(hook.isEnabled());

    EXPECT_TRUE(isNotHooked(foo));
}

// throw invalid_argument if any arugment is nullptr
TEST_F(VftHookTests, InvalidCtorArguments) {
    Foo* cls                                 = nullptr;
    decltype(&VTable<Foo>::echoInt) memberFn = nullptr;
    int (*detour)(void*, int)                = nullptr;

    EXPECT_THROW(VftHook(cls, &VTable<Foo>::echoInt, doubleInt), std::invalid_argument);
    EXPECT_THROW(VftHook(&foo, memberFn, doubleInt), std::invalid_argument);
    EXPECT_THROW(VftHook(&foo, &VTable<Foo>::echoInt, detour), std::invalid_argument);
}

// Test enable/disable member functions
TEST_F(VftHookTests, EnableDisable) {
    VftHook hook{ &foo, &VTable<Foo>::echoInt, doubleInt };
    EXPECT_TRUE(hook.enable());
    EXPECT_TRUE(hook.isEnabled());
    EXPECT_FALSE(hook.enable());

    EXPECT_TRUE(isHooked(foo));

    EXPECT_TRUE(hook.disable());
    EXPECT_FALSE(hook.isEnabled());
    EXPECT_FALSE(hook.disable());

    EXPECT_TRUE(isNotHooked(foo));
}

// Hooks are RAII types, test resource release on destruction
TEST_F(VftHookTests, DestructionFreesResource) {
    {
        VftHook hook{ &foo, &VTable<Foo>::echoInt, doubleInt };
        hook.enable();
    }
    EXPECT_TRUE(isNotHooked(foo));
}

// Test correct ownership transfer and cleanup for move operations
TEST_F(VftHookTests, Move) {

    auto hook = new VftHook{ &foo, &VTable<Foo>::echoInt, doubleInt };
    hook->enable();

    VftHook hook2(std::move(*hook));
    EXPECT_FALSE(hook->isValid());
    delete hook;

    EXPECT_TRUE(isHooked(foo));
}

// Test invocation of original function while hook is enabled
TEST_F(VftHookTests, InvokeOriginal) {
    VftHook hook{ &foo, &VTable<Foo>::echoInt, doubleInt };
    hook.enable();

    EXPECT_EQ(hook.invokeOriginal(nullptr, v), v);
}

std::unique_ptr<IHook> IatHookTests::createHook() {

    static auto proc      = []() { return 0ul; };
    unsigned long (*fp)() = proc;

    return std::make_unique<GetCurrentProcessIdHook_t>("KERNEL32.dll", "GetCurrentProcessId", fp);
}

bool IatHookTests::hookIsActive() {
    auto pid = GetCurrentProcessId();
    return !pid;
}
