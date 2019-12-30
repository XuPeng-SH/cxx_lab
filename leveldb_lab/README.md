# Memo

## Test

1.2 GB/s disk, first time insert, performance will downgrade during background merge

| Key Length | Value Length | Count  |  Time(s) |
| ---------   | -------- | -------- | -------------- |
| 32 | 16 | 1000000  | 3.3 |
| 32 | 2048 | 1000000  | 22.1 |
| 32 | 2097152 | 977  | 10.8 |
| 32 | 2097152*2 | 977/2  | 10.2 |
| 32 | 2097152*4 | 977/4  | 7.8 |
| 32 | 2097152*8 | 977/8  | 6.5 |
| 32 | 2097152*16 | 977/16  | 6.2 |
