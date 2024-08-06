Name: Noah Shi
Email: noahshi@uw.edu

Bonus #3:
Before Optimization:
```
--------------------------------------------------------------------------------
Profile data file 'callgrind.out.1094714' (creator: callgrind-3.21.0)
--------------------------------------------------------------------------------
I1 cache:
D1 cache:
LL cache:
Timerange: Basic block 0 - 352991103
Trigger: Program termination
Profiled target:  ./filesearchshell ./unit_test_indices/bash.idx ./unit_test_indices/books.idx ./unit_test_indices/enron.idx ./unit_test_indices/tiny.idx ./unit_test_indices/wiki.idx (PID 1094714, part 1)
Events recorded:  Ir
Events shown:     Ir
Event sort order: Ir
Thresholds:       99
Include dirs:
User annotated:
Auto-annotation:  on

--------------------------------------------------------------------------------
Ir
--------------------------------------------------------------------------------
3,555,319,471 (100.0%)  PROGRAM TOTALS
```



Snippet from "Utils.cc"
```
  512,060,688 (14.40%)  void CRC32::FoldByteIntoCRC(uint8_t next_byte) {
  426,717,240 (12.00%)    Verify333(finalized_ != true);
1,280,151,720 (36.01%)    crc_state_ = (crc_state_ >> 8) ^ table_[(crc_state_ & 0xFF) ^ next_byte];
  256,030,344 ( 7.20%)  }
```



Snippet from "FileIndexReader.cc"
```
    333,391 ( 0.01%)      while (left_to_read > 0) {
          .                 // STEP 4.
          .                 // You should only need to modify code inside the while loop for
          .                 // this step. Remember that file_ is now unbuffered, so care needs
          .                 // to be put into how the file is sequentially read
  1,666,880 ( 0.05%)        int bytes_read = fread(buf, 1, kBufSize, file_);
 27,003,751 ( 0.76%)  => ???:fread (166,688x)
342,207,232 ( 9.63%)
682,747,584 (19.20%)        crc_obj.FoldBytesIntoCRC(buf, bytes\_read);
2,474,959,992 (69.61%)  => /homes/iws/noahshi/cse333-24sp-noahshi/hw3/Utils.cc:hw3::CRC32::FoldByteIntoCRC(unsigned char) (85,343,448x)
          .                 left_to_read -= bytes_read;
    333,376 ( 0.01%)      }
          .               Verify333(crc_obj.GetFinalCRC() == header_.checksum);
         45 ( 0.00%)    }
```

After reading through the callgrind annotations, it becomes very clear that the program spends a majority of its instructions calling CRC32::FoldBytesIntoCRC.
What is especially wasteful is that while we are calling FoldBytesIntoCRC in the while loop in "FileIndexReader.cc", we spend 426 million instructions repeatedly calling Verify333 on a value we have not changed.
We can also see than in the "FileIndexReader.cc" snippet, fread is called 166,688 times, which is a lot of disk accesses, especially since we unbuffered the file we are reading from earlier.



After Optimization:
```
--------------------------------------------------------------------------------
Profile data file 'callgrind.out.1224523' (creator: callgrind-3.21.0)
--------------------------------------------------------------------------------
I1 cache:
D1 cache:
LL cache:
Timerange: Basic block 0 - 34347524
Trigger: Program termination
Profiled target:  ./filesearchshell unit_test_indices/bash.idx unit_test_indices/books.idx unit_test_indices/enron.idx unit_test_indices/tiny.idx unit_test_indices/wiki.idx (PID 1224523, part 1)
Events recorded:  Ir
Events shown:     Ir
Event sort order: Ir
Thresholds:       99
Include dirs:
User annotated:
Auto-annotation:  on

--------------------------------------------------------------------------------
Ir
--------------------------------------------------------------------------------
1,086,433,615 (100.0%)  PROGRAM TOTALS
```



Snippet from "Utils.cc":
```
        680 ( 0.00%)  void CRC32::FoldBytesIntoCRC(const uint8_t *next_bytes, size_t num_bytes) {
      3,999 ( 0.00%)  => ???:mcount (85x)
        425 ( 0.00%)    Verify333(finalized_ != true);
          .
        255 ( 0.00%)    size_t batches = num_bytes / 8;
        255 ( 0.00%)    uint32_t crc = crc_state_;
          .
 42,672,141 ( 3.93%)    for (size_t i = 0; i < batches; ++i) {
 64,007,574 ( 5.89%)      uint64_t bytes = *(reinterpret_cast<const int64_t*>(&next_bytes[i*8]));
 96,011,361 ( 8.84%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ (bytes & 0xFF)];
117,347,219 (10.80%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ ((bytes >> 8) & 0xFF)];
117,347,219 (10.80%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ ((bytes >> 16) & 0xFF)];
117,347,219 (10.80%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ ((bytes >> 24) & 0xFF)];
117,347,219 (10.80%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ ((bytes >> 32) & 0xFF)];
117,347,219 (10.80%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ ((bytes >> 40) & 0xFF)];
117,347,219 (10.80%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ ((bytes >> 48) & 0xFF)];
117,347,219 (10.80%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ ((bytes >> 56) & 0xFF)];
          .             }
          .
        659 ( 0.00%)    for (size_t i = batches * 8; i < num_bytes; ++i) {
        240 ( 0.00%)      crc = (crc >> 8) ^ table_[(crc & 0xFF) ^ next_bytes[i]];
          .             }
          .
        255 ( 0.00%)    crc_state_ = crc;
        255 ( 0.00%)  }
```



Snippet from "FileIndexReader.cc":
```
185 ( 0.00%)      while (left_to_read > 0) {
  .                 // STEP 4.
  .                 // You should only need to modify code inside the while loop for
  .                 // this step. Remember that file_ is now unbuffered, so care needs
  .                 // to be put into how the file is sequentially read
850 ( 0.00%)        int bytes_read = fread(buf, 1, kBufSize, file_);
14,088 ( 0.00%)  => ???:fread (85x)
  .
595 ( 0.00%)        crc_obj.FoldBytesIntoCRC(buf, bytes_read);
1,024,128,632 (94.27%)  => /homes/iws/noahshi/cse333-24sp-noahshi/hw3/Utils.cc:hw3::CRC32::FoldBytesIntoCRC(unsigned char const*, unsigned long) (85x)
170 ( 0.00%)        left_to_read -= bytes_read;
  .               }
 45 ( 0.00%)      Verify333(crc_obj.GetFinalCRC() == header_.checksum);
354 ( 0.00%)  => /homes/iws/noahshi/cse333-24sp-noahshi/hw3/Utils.cc:hw3::CRC32::GetFinalCRC() (5x)
  .             }
```

To improve all of this, I wrote a new function in "Utils.cc" to fold multiple bytes into the CRC in one function call and I updated "FileIndexReader.cc" to use this new function instead.
This reduces the number of overhead from repeatedly calling FoldByteIntoCRC and it also reduces the number of redundant Verify333 calls made.
The reduction in the number of function calls is also in part due to expanding the buffer that fread writes to from 512 bytes to 1 million bytes. This reduced the number of fread and FoldBytesIntoCRC calls to 85 each.
We can see that these optimizations had a significant impact, reducing the total number of instructions read from 3.56 billion to 1.09 billion.
