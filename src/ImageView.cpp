#include "ImageView.h"
#include "Cast.h"
#include "Define.h"
#include "Exception.h"
#include "Process.h"
#include "StringUtil.h"
#include <Windows.h>
#include <concepts>

namespace {

    template <typename T>
    bool fitsInRange(const uint8_t* begin, const uint8_t* end, const T* obj, size_t size) {
        return begin <= reinterpret_cast<const uint8_t*>(obj) && (reinterpret_cast<const uint8_t*>(obj) + size) < end;
    }

    template <typename T>
    bool fitsInRange(const uint8_t* begin, const uint8_t* end, T* obj) {
        return begin <= reinterpret_cast<const uint8_t*>(obj) && reinterpret_cast<const uint8_t*>(++obj) < end;
    }

} // namespace

B3L::ImageView::ImageView(const uint8_t* data) : imageBase(data) {
}

void B3L::ImageView::validateHeaderStructure(const ImageView& view, size_t maxHeaderSize) {
    auto begin = view.imageBase;
    auto end   = begin + maxHeaderSize;

    // Validate Dos Header
    auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(begin);
    if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        throw std::runtime_error("Invalid Dos signature");

    // Validate Nt Headers
    auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(begin + dosHeader->e_lfanew);
    if(!fitsInRange(begin, end, ntHeaders))
        throw std::runtime_error("Invalid e_lfanew offset");
    if(ntHeaders->Signature != IMAGE_NT_SIGNATURE)
        throw std::runtime_error("Invalid NT signature");

    // Validate Section Headers
    auto sectionHeaders =
    reinterpret_cast<const IMAGE_SECTION_HEADER*>(reinterpret_cast<const uint8_t*>(ntHeaders) + sizeof(ntHeaders->Signature) +
                                                  sizeof(ntHeaders->FileHeader) + ntHeaders->FileHeader.SizeOfOptionalHeader);
    auto sectionHeadersTotalSize = ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

    if(!fitsInRange(begin, end, sectionHeaders, sectionHeadersTotalSize))
        throw std::runtime_error("Invalid section headers");
}

void B3L::ImageView::validateMappedSectionStructure(const ImageView& view) {
    for(int idx = 0; idx < view.sectionCount(); ++idx) {
        auto section             = view.section(idx);
        auto virtualAddressBegin = view.imageBase + section->VirtualAddress;

        // Section memory has been allocated.
        size_t totalRegionSize = 0;
        auto head              = virtualAddressBegin;
        while(totalRegionSize < section->Misc.VirtualSize) {
            MEMORY_BASIC_INFORMATION info;
            if(!VirtualQuery(head, &info, sizeof(MEMORY_BASIC_INFORMATION)) || !(info.State & MEM_COMMIT))
                throw std::runtime_error("Invalid section allocation");

            totalRegionSize += info.RegionSize;
            head += info.RegionSize;
        }
    }
}

std::optional<B3L::ImageView> B3L::ImageView::createFromMappedImage(const uint8_t* data) {
    MEMORY_BASIC_INFORMATION info;
    if(!VirtualQuery(data, &info, sizeof(MEMORY_BASIC_INFORMATION)))
        throw Win32Exception("VirtualQuery");

    ImageView view(data);
    try {
        validateHeaderStructure(view, info.RegionSize);
        validateMappedSectionStructure(view);
    } catch(...) {
        return std::nullopt;
    }
    return view;
}

void* B3L::detail::getImportAddressTableEntry(const std::string& mod, const std::string& fn, int ordinal) {
    // TODO: Should be rewritten to use ImageView and ImportIterator
    LPVOID imageBase = getModuleBaseAddress();
    if(!imageBase)
        return nullptr;

    auto RVAtoVA = [imageBase](uint64_t offset) { return reinterpret_cast<uint8_t*>(imageBase) + offset; };

    auto dosHeaders = rcast<PIMAGE_DOS_HEADER>(imageBase);
    auto ntHeaders  = rcast<PIMAGE_NT_HEADERS>(RVAtoVA(dosHeaders->e_lfanew));

    const IMAGE_DATA_DIRECTORY importsDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    auto importDescriptor = rcast<PIMAGE_IMPORT_DESCRIPTOR>(RVAtoVA(importsDirectory.VirtualAddress));

    while(importDescriptor->Name != NULL) {
        auto libraryName = rcast<char*>(RVAtoVA(importDescriptor->Name));
        if(StringUtil::iequal(libraryName, mod)) {

            auto library = LoadLibraryA(libraryName);
            if(library) {

                auto originalFirstThunk = rcast<PIMAGE_THUNK_DATA>(RVAtoVA(importDescriptor->OriginalFirstThunk));
                auto firstThunk         = rcast<PIMAGE_THUNK_DATA>(RVAtoVA(importDescriptor->FirstThunk));

                while(auto currentOrdinal = originalFirstThunk->u1.Ordinal) {
                    if(IMAGE_SNAP_BY_ORDINAL(currentOrdinal)) {
                        if(IMAGE_ORDINAL(currentOrdinal) == ordinal) {
                            return &firstThunk->u1.Function;
                        }
                    } else {
                        auto name = B3L::rcast<PIMAGE_IMPORT_BY_NAME>(RVAtoVA(originalFirstThunk->u1.AddressOfData));
                        if(StringUtil::iequal(name->Name, fn)) {
                            return &firstThunk->u1.Function;
                        }
                    }
                    ++originalFirstThunk;
                    ++firstThunk;
                }
            }
        }
        importDescriptor++;
    }
    return nullptr;
}