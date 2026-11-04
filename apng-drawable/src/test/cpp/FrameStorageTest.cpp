#include "ApngFrame.h"
#include "ApngImage.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

using namespace apng_drawable;

static std::unique_ptr<ApngFrame> makeFrame(const std::vector<uint32_t> &pixels) {
  auto frame = std::make_unique<ApngFrame>(pixels.size(), 17);
  memcpy(frame->getRawPixels(), pixels.data(), pixels.size() * sizeof(uint32_t));
  return frame;
}

static void checkPixels(const ApngFrame &frame, const std::vector<uint32_t> &expected) {
  const size_t bytes = expected.size() * sizeof(uint32_t);
  std::vector<uint32_t> actual(expected.size() + 1, 0xfeedbeef);
  assert(!frame.copyPixels(nullptr, bytes));
  assert(!frame.copyPixels(actual.data(), bytes - 1));
  assert(frame.copyPixels(actual.data(), bytes));
  assert(memcmp(actual.data(), expected.data(), bytes) == 0);
  assert(actual.back() == 0xfeedbeef);
  assert(frame.getDuration() == 17);
}

static void testRoundTripsAndFallbacks() {
  std::mt19937 random(7381);
  for (size_t count : {1, 1023, 1024, 16384, 396000}) {
    for (int pattern = 0; pattern < 4; ++pattern) {
      std::vector<uint32_t> pixels(count);
      for (size_t i = 0; i < count; ++i) {
        pixels[i] = pattern == 0 ? 0 : pattern == 1 ? 0xffabcdef :
            pattern == 2 ? random() : (i % 100 < 80 ? 0 : random());
      }
      auto storage = std::make_shared<std::vector<char>>();
      std::vector<char> scratch;
      auto frame = makeFrame(pixels);
      frame->compress(storage, scratch);
      checkPixels(*frame, pixels);
      const size_t stored = storage->size();
      if (count < 1024 || pattern == 2) {
        assert(stored == 0);
        assert(frame->getPixelAllocationByteCount() == count * sizeof(uint32_t));
      } else {
        assert(stored > 0 && stored <= count * sizeof(uint32_t) * 3 / 4);
        assert(frame->getPixelAllocationByteCount() == 0);
      }
      frame->compress(storage, scratch);
      assert(storage->size() == stored);
      checkPixels(*frame, pixels);
    }
  }
}

static void testImageStorageAndLifetime() {
  std::mt19937 random(9138);
  auto image = std::make_unique<ApngImage>(16384, 1, 100, 0, true);
  std::vector<std::shared_ptr<ApngFrame>> retainedFrames;
  std::vector<std::vector<uint32_t>> expected;
  size_t rawBytes = 0;
  for (uint32_t i = 0; i < 100; ++i) {
    std::vector<uint32_t> pixels(16384);
    for (size_t j = 0; j < pixels.size(); ++j) {
      pixels[j] = i % 5 == 0 ? random() : (j % 100 < i % 90 ? random() : 0);
    }
    image->setFrame(i, makeFrame(pixels));
    retainedFrames.push_back(image->getFrame(i));
    rawBytes += image->getFrame(i)->getPixelAllocationByteCount();
    expected.push_back(std::move(pixels));
  }
  image->finishDecoding();
  assert(image->getAllFrameByteCount() > rawBytes);
  assert(image->getAllFrameByteCount() < 100 * 16384 * sizeof(uint32_t));
  // Replacing a raw frame releases its allocation; the shared compressed buffer stays counted.
  const auto before = image->getAllFrameByteCount();
  image->setFrame(0, nullptr);
  assert(image->getAllFrameByteCount() == before - 16384 * sizeof(uint32_t));
  // Removing compressed frames does not release their shared storage prematurely.
  const auto after = image->getAllFrameByteCount();
  image->setFrame(1, nullptr);
  assert(image->getAllFrameByteCount() == after);
  image.reset();
  for (size_t i = 0; i < retainedFrames.size(); ++i) {
    checkPixels(*retainedFrames[i], expected[i]);
  }
}

static void testDefaultRemainsUncompressed() {
  ApngImage image(128, 128, 2, 0);
  std::vector<uint32_t> pixels(128 * 128, 0);
  image.setFrame(0, makeFrame(pixels));
  image.setFrame(1, makeFrame(pixels));
  image.finishDecoding();
  assert(image.getAllFrameByteCount() == pixels.size() * sizeof(uint32_t) * 2);
  assert(image.getFrame(0)->getRawPixels() != nullptr);
  checkPixels(*image.getFrame(0), pixels);
}

int main() {
  testRoundTripsAndFallbacks();
  testImageStorageAndLifetime();
  testDefaultRemainsUncompressed();
  std::cout << "Frame storage, bounds, accounting, and shared lifetime passed\n";
}
