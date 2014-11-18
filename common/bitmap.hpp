#ifndef _COMMON_BITMAP_HPP_
#define _COMMON_BITMAP_HPP_

#include <omp.h>
#include <stdio.h>
#include <memory.h>

#include <sstream>
#include <vector>
#include <string>

namespace common {

class Bitmap {
  
  public:
    Bitmap() : threads_(1), data_(NULL) {}

    Bitmap(uint32_t threads) : data_(NULL) {
      SetThreads(threads);
    }

    virtual ~Bitmap() {
      delete [] data_;
      data_ = NULL;
    }

    struct Header {
      Header () : width(0), size(0), height(0) {}
      uint32_t width;   // size in byte
      uint32_t size;    // size in bit
      uint32_t height;  
    };

    std::string ToString() const {
      std::stringstream buffer;
      buffer << header_.height << " * " << header_.size << std::endl;
      for (uint32_t i = 0; i < header_.height; ++i) {
        buffer << i << " ";
        for (uint32_t j = 0; j < header_.size; ++j) {
          buffer << GetBit(i, j); 
        }
        buffer << std::endl;
      }
      return buffer.str();
    }


    void SetThreads(uint32_t threads) {
      uint32_t max_threads = omp_get_num_procs() / 4;
      this->threads_ = (threads > max_threads) ? max_threads : threads;
      this->threads_ = (this->threads_ < 1) ? 1 : this->threads_;
    }

    uint32_t GetWidth() const {
      return header_.width;
    }

    uint32_t GetSize() const {
      return header_.size;
    }

    uint32_t GetHeight() const {
      return header_.height;
    }

    void Clear() {
      header_.width = 0;
      header_.size = 0;
      header_.height = 0;
      delete [] data_;
      data_ = NULL;
    }

    bool Generate(const std::vector<std::vector<uint32_t> > &cols, uint32_t height) {
      Clear();
      header_.size = cols.size();
      header_.width = ((header_.size >> 6) + ((header_.size & 63) > 0 ? 1 : 0)) << 3;
      header_.height = height;

      data_ = new uint8_t[header_.width * header_.height];
      memset(data_, 0, header_.width * header_.height);

      for (uint32_t i = 0; i < cols.size(); ++i) {
        for (uint32_t j = 0; j < cols[i].size(); ++j) {
          if (cols[i][j] >= header_.height)
            return false;
          SetBit(cols[i][j], i);
        }
      }
      return true;
    }

    int Load(FILE* in) {
      Clear();
      if (in == NULL)
        return -1;
      if (fread(&header_, sizeof(header_), 1, in) == 0)
        return 1;
      data_ = new uint8_t[header_.width * header_.height];
      if (fread(data_, sizeof(uint8_t), header_.width * header_.height, in) == 0)
        return 1;
      return 0;
    }

    int64_t Dump(FILE* out) const {
      if (out == NULL)
        return -1;
      int64_t file_size = 0;
      fwrite(&header_, sizeof(header_), 1, out);
      file_size += sizeof(header_);
      fwrite(data_, sizeof(uint8_t), header_.width * header_.height, out);
      file_size += (sizeof(uint8_t) * header_.width * header_.height);
      return file_size;
    }

    // result = bitmap[first] & bitmap[second];
    void And(uint32_t first, uint32_t second, uint8_t* result) const {
      uint64_t* first64 = (uint64_t*)(data_ + first * header_.width);
      uint64_t* second64 = (uint64_t*)(data_ + second * header_.width);
      uint64_t* result64 = (uint64_t*)result;
      int bound = GetBound();
#pragma omp parallel for num_threads(threads_)
      for (int i = 0; i < bound; ++i)
        result64[i] = first64[i] & second64[i];
    }

    void Or(uint32_t first, uint32_t second, uint8_t* result) const {
      uint64_t* first64 = (uint64_t*)(data_ + first * header_.width);
      uint64_t* second64 = (uint64_t*)(data_ + second * header_.width);
      uint64_t* result64 = (uint64_t*)result;
      int bound = GetBound();
#pragma omp parallel for num_threads(threads_)
      for (int i = 0; i < bound; ++i)
        result64[i] = first64[i] | second64[i];
    }

    void And(uint32_t first, uint8_t* second, uint8_t* result) const {
      uint64_t* first64 = (uint64_t*)(data_ + first * header_.width);
      uint64_t* second64 = (uint64_t*)second;
      uint64_t* result64 = (uint64_t*)result;
      int bound = GetBound();
#pragma omp parallel for num_threads(threads_)
      for (int i = 0; i < bound; ++i)
        result64[i] = first64[i] & second64[i];
    }

    void Or(uint32_t first, uint8_t* second, uint8_t* result) const {
      uint64_t* first64 = (uint64_t*)(data_ + first * header_.width);
      uint64_t* second64 = (uint64_t*)second;
      uint64_t* result64 = (uint64_t*)result;
      int bound = GetBound();
#pragma omp parallel for num_threads(threads_)
      for (int i = 0; i < bound; ++i)
        result64[i] = first64[i] | second64[i];
    }

    void And(uint8_t* first, uint8_t* second, uint8_t* result) const {
      uint64_t* first64 = (uint64_t*)first;
      uint64_t* second64 = (uint64_t*)second;
      uint64_t* result64 = (uint64_t*)result;
      int bound = GetBound();
#pragma omp parallel for num_threads(threads_)
      for (int i = 0; i < bound; ++i)
        result64[i] = first64[i] & second64[i];
    }

    void Or(uint8_t* first, uint8_t* second, uint8_t* result) const {
      uint64_t* first64 = (uint64_t*)first;
      uint64_t* second64 = (uint64_t*)second;
      uint64_t* result64 = (uint64_t*)result;
      int bound = GetBound();
#pragma omp parallel for num_threads(threads_)
      for (int i = 0; i < bound; ++i)
        result64[i] = first64[i] | second64[i];
    } 


    int Count(const uint8_t* mask) const {
      int sum = 0;
#pragma omp parallel for num_threads(threads_) reduction(+: sum)
      for (int i = 0; i < static_cast<int>(header_.width); ++i) {
        uint8_t s = mask[i];
        for (int j = Pop(s); j >= 0; j = Pop(s)) {
          ++sum;
        }
      }
      return sum;
    }

    int Count(uint32_t index) const {
      int sum = 0;
      if (index < header_.height) {
#pragma omp parallel for num_threads(threads_) reduction(+: sum)
        for (int i = 0; i < static_cast<int>(header_.width); ++i) {
          uint8_t s = data_[index * header_.width + i];
          for (int j = Pop(s); j >= 0; j = Pop(s)) {
            ++sum;
          }
        }
      }
      return sum;
    }

  protected:
    
    int Pop(uint8_t &x) const {
      if (!x) return -1;
      uint8_t del = x & (x ^ (x - 1));
      x -= del;
      return __builtin_ctz(del);
    }

    uint32_t threads_;

  private:

    int GetBound() const {
      return static_cast<int>((header_.width >> 3) + (((header_.width & 7) > 0) ? 1 : 0));
    }
    
    // return bitmap[row][col]
    uint32_t GetBit(uint32_t row, uint32_t col) const {
      return data_[row * header_.width + (col >> 3)] >> (7 - (col & 7)) & 1;
    }

    // bitmap[row][col] = 1
    void SetBit(uint32_t row, uint32_t col) {
      data_[row * header_.width + (col >> 3)] |= (1 << (7 - (col & 7)));
    }

    Header header_;
    uint8_t* data_;

};

}
#endif
