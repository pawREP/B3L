#include "B3L/ImageView.h"
#include "B3L/Process.h"
#include <Windows.h>
#include <gtest/gtest.h>

TEST(ImageViewTests, Construct) {
    auto moduleBase = B3L::getModuleBaseAddress();
    auto imageView  = B3L::ImageView::createFromMappedImage(moduleBase);
    EXPECT_TRUE(imageView.has_value());
}

TEST(ImageViewTests, ImportIterator) {
    auto moduleBase = B3L::getModuleBaseAddress();
    auto imageView  = B3L::ImageView::createFromMappedImage(moduleBase);
    EXPECT_TRUE(imageView.has_value());

    const auto begin = imageView->importsBegin();
    const auto end   = imageView->importsEnd();

    // Iterate full range
    auto head = begin;
    while(head != end) {
        ++head;
    }

    // Find import by name
    head = begin;
    while(head != end) {
        if(strcmp(head->name(), "VirtualAlloc") == 0)
            break;
        ++head;
    }
    EXPECT_TRUE(head != end);
}