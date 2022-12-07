#pragma once
#include "Define.h"
#include "ImageView.h"
#include "Memory.h"
#include <stdexcept>
#include <string>

namespace B3L {

    class IHook {
    public:
        virtual ~IHook() = default;

        virtual bool enable()                 = 0;
        virtual bool disable()                = 0;
        virtual bool isEnabled() const        = 0;
        virtual bool isValid() const          = 0;
        virtual std::unique_ptr<IHook> move() = 0;

        template <typename Sig>
        auto originalTarget() const {
            static_assert(std::is_function_v<Sig>);
            return reinterpret_cast<Sig*>(_originalTarget());
        }

        template <typename Sig, typename... Args>
        auto invokeOriginal(Args&&... args) const {
            static_assert(std::is_function_v<Sig>);
            return originalTarget<Sig>()(std::forward<Args>(args)...);
        }

    protected:
        IHook() = default;

        virtual void* _originalTarget() const = 0;
    };

    template <typename TargetProcT>
    class VftHook : public IHook {
        B3L_MAKE_NONCOPYABLE(VftHook);

    public:
        template <typename Klass, typename Ret, typename VTable, typename... Args>
        VftHook(Klass* instance, Ret (*VTable::*function)(Args...), Ret (*proc)(Args...));

        ~VftHook();
        VftHook(VftHook&&) noexcept;
        VftHook& operator=(VftHook&&) noexcept;

        bool enable() override;
        bool disable() override;
        bool isEnabled() const override;
        bool isValid() const override;
        std::unique_ptr<IHook> move() override;

        template <typename... Args>
        auto invokeOriginal(Args&&... args) const {
            return IHook::invokeOriginal<std::remove_pointer_t<TargetProcT>>(std::forward<Args>(args)...);
        }

    protected:
        void* _originalTarget() const override;

    private:
        TargetProcT* vftEntry  = nullptr;
        TargetProcT originalFn = nullptr;
        TargetProcT detourFn   = nullptr;

        void invalidate();
    };

    template <typename Klass, typename Ret, typename VTable, typename... Args>
    VftHook(Klass* instance, Ret (*VTable::*function)(Args...), Ret (*proc)(Args...)) -> VftHook<Ret (*)(Args...)>;

    template <typename TargetProcT>
    class IatHook : public IHook {
        B3L_MAKE_NONCOPYABLE(IatHook);

        static_assert(std::is_pointer_v<TargetProcT>);
        static_assert(std::is_function_v<std::remove_pointer_t<TargetProcT>>);

    public:
        IatHook(const std::string& moduleName, const std::string& name, TargetProcT proc);
        IatHook(const std::string& moduleName, int ordinal, TargetProcT proc);
        ~IatHook();

        IatHook(IatHook&& other) noexcept;
        IatHook& operator=(IatHook&& other) noexcept;

        template <typename... Args>
        auto invokeOriginal(Args&&... args) const {
            return IHook::invokeOriginal<std::remove_pointer_t<TargetProcT>>(std::forward<Args>(args)...);
        }

        // IHook
        bool enable() override;
        bool disable() override;
        bool isEnabled() const override;
        // A hook instance is considered valid if it is associated with a valid IAT pointer pointing to one of the two targets
        // established during construction. This function is mainly intended for debugging to check whether another library overrides the installed hook.
        bool isValid() const override;
        std::unique_ptr<IHook> move() override;

    protected:
        void* _originalTarget() const override;

    private:
        IatHook() = default;
        IatHook(const std::string& moduleName, const std::string& name, int ordinal, TargetProcT proc);

        void invalidate();

        TargetProcT* iatEntry = nullptr;
        TargetProcT _original = nullptr;
        TargetProcT _detour   = nullptr;
    };

    template <typename TargetProcT>
    inline IatHook<TargetProcT>::~IatHook() {
        if(!isValid())
            return;

        disable();
    }

    template <typename TargetProcT>
    inline IatHook<TargetProcT>::IatHook(IatHook&& other) noexcept : IatHook<TargetProcT>::IatHook() {
        *this = std::move(other);
    }

    template <typename TargetProcT>
    inline IatHook<TargetProcT>& IatHook<TargetProcT>::operator=(IatHook&& other) noexcept {
        if(isValid())
            disable();

        iatEntry  = other.iatEntry;
        _original = other._original;
        _detour   = other._detour;

        other.invalidate();

        return *this;
    }

    template <typename TargetProcT>
    inline bool IatHook<TargetProcT>::enable() {
        assert(isValid());

        if(isEnabled())
            return false;

        Memory::swapFunctionPointer(iatEntry, _detour);
        return true;
    }

    template <typename TargetProcT>
    inline bool IatHook<TargetProcT>::disable() {
        assert(isValid());

        if(!isEnabled())
            return false;

        Memory::swapFunctionPointer(iatEntry, _original);
        return true;
    }

    template <typename TargetProcT>
    inline bool IatHook<TargetProcT>::isEnabled() const {
        assert(isValid());
        return *iatEntry == _detour;
    }

    template <typename TargetProcT>
    inline bool IatHook<TargetProcT>::isValid() const {
        if(!iatEntry)
            return false;

        void* currentPtr = *reinterpret_cast<intptr_t**>(iatEntry);
        return (currentPtr == _original) || (currentPtr == _detour);
    }

    template <typename TargetProcT>
    inline std::unique_ptr<IHook> IatHook<TargetProcT>::move() {
        return std::make_unique<IatHook<TargetProcT>>(std::move(*this));
    }

    template <typename TargetProcT>
    inline void* IatHook<TargetProcT>::_originalTarget() const {
        return reinterpret_cast<void*>(_original);
    }

    template <typename TargetProcT>
    inline IatHook<TargetProcT>::IatHook(const std::string& moduleName, const std::string& name, TargetProcT proc)
    : IatHook(moduleName, name, -1, proc) {
    }

    template <typename TargetProcT>
    inline IatHook<TargetProcT>::IatHook(const std::string& moduleName, int ordinal, TargetProcT proc)
    : IatHook(moduleName, "", ordinal, proc) {
    }

    template <typename TargetProcT>
    inline IatHook<TargetProcT>::IatHook(const std::string& moduleName, const std::string& name, int ordinal, TargetProcT proc) {
        if(moduleName.empty())
            throw std::invalid_argument("Empty module name");
        if(!proc)
            throw std::invalid_argument("Hook proc can't be nullptr");
        if(name.empty() && ordinal < 0)
            throw std::invalid_argument("Invalid import ordinal");

        _detour  = proc;
        iatEntry = getImportAddressTableEntry<TargetProcT>(moduleName, name, ordinal);
        if(!iatEntry)
            throw std::runtime_error("IAT entry not found");

        _original = *iatEntry;
    }

    template <typename TargetProcT>
    inline void IatHook<TargetProcT>::invalidate() {
        iatEntry = nullptr;
        assert(!isValid()); // Double checking that invalidate() and isValid() stay in sync;
    }

    template <typename TargetProcT>
    inline VftHook<TargetProcT>::~VftHook() {
        if(isValid())
            disable();
    }

    template <typename TargetProcT>
    inline VftHook<TargetProcT>::VftHook(VftHook&& other) noexcept
    : vftEntry(other.vftEntry), originalFn(other.originalFn), detourFn(other.detourFn) {
        other.invalidate();
    }

    template <typename TargetProcT>
    inline VftHook<TargetProcT>& VftHook<TargetProcT>::operator=(VftHook&& other) noexcept {
        disable();

        vftEntry   = other.vftEntry;
        originalFn = other.originalFn;
        detourFn   = other.detourFn;

        other.invalidate();

        return *this;
    }

    template <typename TargetProcT>
    inline bool VftHook<TargetProcT>::enable() {
        if(isEnabled())
            return false;

        Memory::writeProtectedMemory(vftEntry, detourFn);
        return true;
    }

    template <typename TargetProcT>
    inline bool VftHook<TargetProcT>::disable() {
        if(!isEnabled())
            return false;

        Memory::writeProtectedMemory(vftEntry, originalFn);
        return true;
    }

    template <typename TargetProcT>
    inline bool VftHook<TargetProcT>::isEnabled() const {
        return *vftEntry == detourFn;
    }

    template <typename TargetProcT>
    inline bool VftHook<TargetProcT>::isValid() const {
        if(!vftEntry)
            return false;

        return (*vftEntry == originalFn) || (*vftEntry == detourFn);
    }

    template <typename TargetProcT>
    inline std::unique_ptr<IHook> VftHook<TargetProcT>::move() {
        return std::make_unique<VftHook<TargetProcT>>(std::move(*this));
    }

    template <typename TargetProcT>
    inline void* VftHook<TargetProcT>::_originalTarget() const {
        return reinterpret_cast<void*>(originalFn);
    }

    template <typename TargetProcT>
    inline void VftHook<TargetProcT>::invalidate() {
        vftEntry = nullptr;
        assert(!isValid());
    }

    template <typename TargetProcT>
    template <typename Klass, typename Ret, typename VTable, typename... Args>
    inline VftHook<TargetProcT>::VftHook(Klass* instance, Ret (*VTable::*function)(Args...), Ret (*proc)(Args...))
    : detourFn(proc) {
        static_assert(std::is_polymorphic_v<Klass>);
        static_assert(std::is_same_v<Ret (*)(Args...), TargetProcT>);

        if(!instance || !function || !proc)
            throw std::invalid_argument("Arguments can't be nullptr");

        auto vft   = *reinterpret_cast<VTable**>(instance);
        vftEntry   = &(vft->*function);
        originalFn = *vftEntry;
    }

} // namespace B3L