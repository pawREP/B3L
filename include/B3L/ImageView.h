#pragma once
#include "Cast.h"
#include <Windows.h>
#include <cassert>
#include <cstddef>
#include <optional>
#include <span>

namespace B3L {
    class ImageView {
    public:
        class SectionView;
        class Import;
        class ImportIterator;

        // Creates structurally validated ImageView from mapped Image. Throws on failure.
        [[nodiscard]] static std::optional<ImageView> createFromMappedImage(const uint8_t* data);

        [[nodiscard]] inline uintptr_t baseAddress() const noexcept {
            return rcast<uintptr_t>(imageBase);
        }

        [[nodiscard]] inline const IMAGE_DOS_HEADER* dosHeader() const noexcept {
            return rcast<const IMAGE_DOS_HEADER*>(imageBase);
        }

        [[nodiscard]] inline const IMAGE_NT_HEADERS* ntHeaders() const noexcept {
            return rcast<const IMAGE_NT_HEADERS*>(imageBase + dosHeader()->e_lfanew);
        }

        [[nodiscard]] inline const IMAGE_OPTIONAL_HEADER* optionalHeader() const noexcept {
            return &ntHeaders()->OptionalHeader;
        }

        [[nodiscard]] inline const IMAGE_FILE_HEADER* fileHeader() const noexcept {
            return &ntHeaders()->FileHeader;
        }

        [[nodiscard]] inline const IMAGE_DATA_DIRECTORY* dataDirectory(int index) const noexcept {
            assert(index >= 0);
            assert(index < IMAGE_NUMBEROF_DIRECTORY_ENTRIES);

            return &optionalHeader()->DataDirectory[index];
        }

        [[nodiscard]] inline int sectionCount() const noexcept {
            return fileHeader()->NumberOfSections;
        }

        // Returns pointer to first section header. Iterating up to sectionCount() is guaranteed to be safe.
        [[nodiscard]] inline const IMAGE_SECTION_HEADER* sections() const noexcept {
            const auto nth              = ntHeaders();
            const auto sectionHeaderOff = rcast<const uint8_t*>(nth) + sizeof(nth->Signature) +
                                          sizeof(nth->FileHeader) + nth->FileHeader.SizeOfOptionalHeader;

            return rcast<const IMAGE_SECTION_HEADER*>(sectionHeaderOff);
        }

        [[nodiscard]] inline const IMAGE_SECTION_HEADER* section(int index) const noexcept {
            const auto cnt = sectionCount();
            if(index < 0 || index >= cnt)
                return nullptr;

            return sections() + index;
        }

        [[nodiscard]] std::span<const uint8_t> sectionData(int index) const noexcept {
            if(sectionCount() <= index)
                return {};

            auto header = section(index);
            auto begin  = imageBase + header->VirtualAddress;
            auto end    = begin + header->Misc.VirtualSize;

            return { begin, end };
        }

        [[nodiscard]] inline uint32_t timestamp() const noexcept {
            return fileHeader()->TimeDateStamp;
        }

        [[nodiscard]] inline ImportIterator importsBegin() const noexcept;
        [[nodiscard]] inline ImportIterator importsEnd() const noexcept;

        template <typename To>
        [[nodiscard]] inline To RVAtoVA(uint64_t rva) const noexcept {
            static_assert(std::is_pointer_v<To>);
            return reinterpret_cast<To>(imageBase + rva);
        }

    private:
        explicit ImageView(const uint8_t* data);

        // Validates that the image headers are structurally valid. Data may still be invalid.
        static void validateHeaderStructure(const ImageView& view, size_t maxHeaderSize);

        // Validates that virtual memory of the correct size and at the correct location has been allocated for all sections as described in the corresponding section headers.
        static void validateMappedSectionStructure(const ImageView& view);

        const uint8_t* imageBase;
    };

    class ImageView::Import {
    public:
        [[nodiscard]] bool importedByName() const noexcept {
            return !importedByOrdinal();
        }

        [[nodiscard]] bool importedByOrdinal() const noexcept {
            return IMAGE_SNAP_BY_ORDINAL(originalFirstThunk->u1.Ordinal);
        }

        [[nodiscard]] const char* moduleName() const noexcept {
            return image->RVAtoVA<const char*>(importDescriptor->Name);
        }

        [[nodiscard]] const char* name() const noexcept {
            if(!importedByName())
                return "";

            auto importByName = image->RVAtoVA<const IMAGE_IMPORT_BY_NAME*>(originalFirstThunk->u1.AddressOfData);
            return importByName->Name;
        }

        [[nodiscard]] int ordinal() const noexcept {
            if(!importedByOrdinal())
                return -1;

            return IMAGE_ORDINAL(originalFirstThunk->u1.Ordinal);
        }

        [[nodiscard]] const uintptr_t* IATEntryAddress() const noexcept {
            return rcast<const uintptr_t*>(&firstThunk->u1.Function);
        }

        [[nodiscard]] auto operator<=>(const Import&) const noexcept = default;

    private:
        friend class ImageView::ImportIterator;

        const IMAGE_IMPORT_DESCRIPTOR* importDescriptor = nullptr;
        const IMAGE_THUNK_DATA* originalFirstThunk      = nullptr;
        const IMAGE_THUNK_DATA* firstThunk              = nullptr;
        const ImageView* image                          = nullptr;
    };

    class ImageView::ImportIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Import;
        using pointer           = value_type*;
        using reference         = value_type&;

        explicit ImportIterator(const ImageView& image) {
            const auto importDir    = image.dataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT);
            desc.importDescriptor   = image.RVAtoVA<const IMAGE_IMPORT_DESCRIPTOR*>(importDir->VirtualAddress);
            desc.originalFirstThunk = image.RVAtoVA<const IMAGE_THUNK_DATA*>(desc.importDescriptor->OriginalFirstThunk);
            desc.firstThunk         = image.RVAtoVA<const IMAGE_THUNK_DATA*>(desc.importDescriptor->FirstThunk);
            desc.image              = &image;
        }
        ImportIterator()                                     = default;
        ImportIterator(const ImportIterator&)                = default;
        ImportIterator(ImportIterator&&) noexcept            = default;
        ~ImportIterator()                                    = default;
        ImportIterator& operator=(const ImportIterator&)     = default;
        ImportIterator& operator=(ImportIterator&&) noexcept = default;

        ImportIterator& operator++();
        ImportIterator operator++(int);

        const value_type& operator*() const;
        const value_type* operator->();

        friend bool operator==(const ImportIterator& lhs, const ImportIterator& rhs);
        friend bool operator!=(const ImportIterator& lhs, const ImportIterator& rhs);

        friend void swap(ImportIterator& lhs, ImportIterator& rhs);

    private:
        Import desc{};
    };

    inline bool operator==(const ImageView::ImportIterator& lhs, const ImageView::ImportIterator& rhs) {
        return lhs.desc == rhs.desc;
    }

    inline bool operator!=(const ImageView::ImportIterator& lhs, const ImageView::ImportIterator& rhs) {
        return lhs.desc != rhs.desc;
    }

    inline void swap(ImageView::ImportIterator& lhs, ImageView::ImportIterator& rhs) {
        std::swap(lhs.desc, rhs.desc);
    }

    inline ImageView::ImportIterator& ImageView::ImportIterator::operator++() {
        ++desc.originalFirstThunk;
        ++desc.firstThunk;

        // End of current import descriptor reached
        if(!desc.originalFirstThunk->u1.Ordinal) {
            desc.importDescriptor++;
            if(desc.importDescriptor->Name) {
                desc.originalFirstThunk = desc.image->RVAtoVA<const IMAGE_THUNK_DATA*>(desc.importDescriptor->OriginalFirstThunk);
                desc.firstThunk = desc.image->RVAtoVA<const IMAGE_THUNK_DATA*>(desc.importDescriptor->FirstThunk);
            } else {
                desc.importDescriptor   = nullptr;
                desc.originalFirstThunk = nullptr;
                desc.firstThunk         = nullptr;
                desc.image              = nullptr;
            }
        }
        return *this;
    }

    inline ImageView::ImportIterator ImageView::ImportIterator::operator++(int) {
        ImageView::ImportIterator old;
        ++(*this);
        return old;
    }

    inline const ImageView::ImportIterator::value_type& ImageView::ImportIterator::operator*() const {
        return desc;
    }
    inline const ImageView::ImportIterator::value_type* ImageView::ImportIterator::operator->() {
        return &desc;
    }

    static_assert(std::forward_iterator<ImageView::ImportIterator>);

    [[nodiscard]] inline ImageView::ImportIterator ImageView::importsBegin() const noexcept {
        return ImageView::ImportIterator{ *this };
    }
    [[nodiscard]] inline ImageView::ImportIterator ImageView::importsEnd() const noexcept {
        return {};
    }

    namespace detail { // TODO: Reimplement with IMageView
        [[nodiscard]] void* getImportAddressTableEntry(const std::string& module, const std::string& fn, int ordinal = 0);
    } // namespace detail

    // Returns the address of the import address table entry corresponding to the passed arguments.
    // Returns nullptr if the entry doesn't exist or when it can't be found
    template <typename T>
    [[nodiscard]] T* getImportAddressTableEntry(const std::string& module, const std::string& fn, int ordinal = 0) {
        static_assert(std::is_pointer_v<T>);

        return reinterpret_cast<T*>(detail::getImportAddressTableEntry(module, fn, ordinal));
    }

} // namespace B3L