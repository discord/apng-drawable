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

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace apng_drawable {

class ApngFrame {
 public:
  ApngFrame(size_t pixelCount, size_t duration);
  ApngFrame() = delete;

  uint32_t *getRawPixels() const { return mPixels.get(); }
  size_t getDuration() const { return mDuration; }
  // Shared compressed storage is counted once by ApngImage.
  size_t getPixelAllocationByteCount() const { return mPixels ? mByteCount : 0; }
  void compress(const std::shared_ptr<std::vector<char>> &storage, std::vector<char> &scratch);
  bool copyPixels(void *destination, size_t capacity) const;
 private:
  std::unique_ptr<uint32_t[]> mPixels;
  size_t mDuration;
  size_t mByteCount;
  // Offsets survive growth of the shared buffer while subsequent frames decode.
  std::shared_ptr<const std::vector<char>> mCompressedPixels;
  size_t mCompressedOffset = 0;
  int mCompressedSize = 0;

  // Do not copy this object.
  ApngFrame(const ApngFrame &ref);
  ApngFrame &operator=(const ApngFrame &ref);
};
}
