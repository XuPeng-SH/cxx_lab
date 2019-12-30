# RocksDB Memo

## Column Family

### [Flow](./cf.cpp)
* List column families' names first
* Call different DB::Open() base column families list results.

### Drop Column Family
`default` CF cannot be dropped.

### ColumnFamilyHandle ID
The ID is monotonically increasing. `default` CF ID is 0.
```js
1. Create CF "CF1" --> 1
2. Create CF "CF2" --> 2
3. Drop CF "CF1"
4. Create CF "CF1" --> 3
```
