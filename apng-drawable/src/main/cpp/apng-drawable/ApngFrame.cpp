//
// Copyright 2018 LINE Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "ApngFrame.h"
#include "lz4.h"
#include <cstring>
#include <new>
#include <stdexcept>

namespace apng_drawable {

ApngFrame::ApngFrame(size_t pixelCount, size_t duration)
    : mDuration(duration), mByteCount(pixelCount * sizeof(uint32_t)) {
  mPixels = std::make_unique<uint32_t[]>(pixelCount);
}

void ApngFrame::compress(const std::shared_ptr<std::vector<char>> &storage,
                         std::vector<char> &scratch) {
  if (!storage || !mPixels || mByteCount < 4096 || mByteCount > LZ4_MAX_INPUT_SIZE) {
    return;
  }
  const int size = static_cast<int>(mByteCount);
  // Only retain compression when it saves at least 25% of the pixel storage.
  const int capacity = size - size / 4;
  try {
    scratch.resize(capacity);
    const int compressedSize = LZ4_compress_default(
        reinterpret_cast<const char *>(mPixels.get()), scratch.data(), size, capacity);
    if (compressedSize <= 0) {
      return;
    }
    const size_t offset = storage->size();
    storage->insert(storage->end(), scratch.data(), scratch.data() + compressedSize);
    mCompressedPixels = storage;
    mCompressedOffset = offset;
    mCompressedSize = compressedSize;
    mPixels.reset();
  } catch (const std::bad_alloc &) {
    // Allocation failure leaves the original pixels intact.
  } catch (const std::length_error &) {
    // The shared buffer cannot grow any further; retain this frame uncompressed.
  }
}

bool ApngFrame::copyPixels(void *destination, size_t capacity) const {
  if (!destination || capacity < mByteCount) {
    return false;
  }
  if (mCompressedPixels) {
    return LZ4_decompress_safe(
        mCompressedPixels->data() + mCompressedOffset,
        static_cast<char *>(destination), mCompressedSize, static_cast<int>(mByteCount)) ==
        static_cast<int>(mByteCount);
  }
  memcpy(destination, mPixels.get(), mByteCount);
  return true;
}
}
