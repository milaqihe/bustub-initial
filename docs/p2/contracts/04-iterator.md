# P2 成员四接口约定：IndexIterator / Begin / Begin(key) / End

- 提出人：吴雅卓（`WUCROW2006`），成员四
- 状态：草案 v0.1，待成员一、成员二、成员五分别确认
- 建议存放路径：`docs/p2/contracts/04-iterator.md`
- 对应 `docs/p2/INTERFACES.md` 第 51 行「迭代器构造、结束态、复制/移动及页面资源归属，由成员四提出，与成员五确认」

## 0 三项决定

| 决定 | 内容 | 理由 |
| --- | --- | --- |
| 结束态 | `page_ == nullptr`，即 `page_id_ == INVALID_PAGE_ID` | `End()` 需要与任意迭代器比较，该表示不依赖具体数据，且在整棵树内唯一 |
| 页面资源 | 迭代器持有当前叶子页的 pin，析构时 Unpin | 迭代器生命周期可能长于 `Begin()` 调用，期间页面可能被缓冲池换出 |
| 锁 | 迭代器不获取锁，仅持有 pin；读锁由成员五在 Fetch、Unpin 外层获取与释放 | 单线程阶段无锁；并发阶段锁顺序须与删除合并一致，不宜各自实现 |

## 1 向成员一提出的接口需求：叶子页缺少元素读取方法

现状（`src/include/storage/page/b_plus_tree_leaf_page.h`）：

```cpp
void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = LEAF_PAGE_SIZE);
auto GetNextPageId() const -> page_id_t;
void SetNextPageId(page_id_t next_page_id);
auto KeyAt(int index) const -> KeyType;      // 按值返回
private:
  page_id_t next_page_id_;
  MappingType array_[1];                     // 私有，外部无法访问
```

需新增：

```cpp
auto GetItem(int index) const -> const MappingType &;   // 必需
```

理由：`IndexIterator::operator*()` 的签名为 `auto operator*() -> const MappingType &;`，要求返回页内元素的引用。`KeyAt` 按值返回，无法提供引用；`array_` 为 private，迭代器无法读取 value。（`MappingType` 在 `src/include/storage/page/b_plus_tree_page.h:23` 定义为宏 `std::pair<KeyType, ValueType>`。）

若不希望暴露 `array_`，也可提供 `auto ValueAt(int index) const -> ValueType;`，但此时 `operator*` 须改为返回值类型，等于变更公开签名。按课程骨架保持 `const MappingType &`，因此建议采用 `GetItem`。

另需确认：`Init()` 必须将 `next_page_id_` 置为 `INVALID_PAGE_ID`。新建叶子页若保留上一使用者的页号，链尾将指向无效页，扫描时可能读到其他索引的数据。此类错误在 ASan 下通常不报错，表现为扫描结果条目多于实际写入数量。

`KeyAt` 保持现有签名，成员二、成员三正在使用。

## 2 向成员二提出的接口需求：叶子页定位方法

```cpp
// 返回已 Fetch 且已 pin 的 Page*，未加锁；空树返回 nullptr
// leftmost = true 时忽略 key，沿第 0 个 child 下行，供 Begin() 使用
auto FindLeafPage(const KeyType &key, bool leftmost = false) -> Page *;
```

三项约定：

1. 返回值已 pin，可直接 `reinterpret_cast<LeafPage *>(page->GetData())` 使用。
2. 调用方负责 Unpin，并发阶段还包括 RUnlatch。谁 Fetch 谁 Unpin，不引入引用计数。
3. 空树返回 `nullptr`，不返回根页。

成员四承诺只调用该方法，不另行实现遍历。`FindLeafPage` 若改名或调整参数，需提前通知，否则 `Begin` 与 `Begin(key)` 须同步修改。

## 3 IndexIterator 的成员与语义

```cpp
private:
  Page *page_{nullptr};             // 当前叶子页，持有 pin；结束态为 nullptr
  page_id_t page_id_{INVALID_PAGE_ID};
  int index_{0};                    // 当前项在页内的下标
  BufferPoolManager *bpm_{nullptr}; // 用于跨页 Fetch 与析构 Unpin
```

构造：

```cpp
IndexIterator();                                        // 结束态
IndexIterator(BufferPoolManager *bpm, Page *page, int index);
```

| 成员 | 语义 | 实现要点 |
| --- | --- | --- |
| `IsEnd()` | `page_ == nullptr` | 骨架中为非 const 的 `auto IsEnd() -> bool;`，不可在 const 的 `operator==` 中调用，应直接比较成员 |
| `operator*()` | 返回 `const MappingType &` | `reinterpret_cast<LeafPage *>(page_->GetData())->GetItem(index_)` |
| `operator++()` | 前进一项，必要时跨页 | `index_ + 1`；若等于 `leaf->GetSize()` 则取 `GetNextPageId()`，无效则转入结束态并释放当前页 |
| `operator==` | 判定是否指向同一元素 | 两者均为结束态时返回真；否则比较 `page_id_ == o.page_id_ && index_ == o.index_` |
| `operator!=` | 取反 | 实现为 `!(*this == o)`，不重复实现 |
| 析构 | Unpin | `if (page_ != nullptr) bpm_->UnpinPage(page_id_, false);` |
| 拷贝 | 禁止 | `IndexIterator(const IndexIterator &) = delete;`。否则两个对象对同一页 Unpin 两次，pin 计数减少两次 |
| 移动 | 允许 | 转移四个成员，源对象置 `page_ = nullptr; page_id_ = INVALID_PAGE_ID;` |

### 编译期注意事项

1. `operator==` 与 `operator!=` 现定义在 `index_iterator.h` 第 35 至 37 行，函数体为 `throw`。修改时改头文件，不在 `.cpp` 中查找。
2. `IsEnd()` 为非 const 成员，在 const 的 `operator==` 中调用会导致编译失败。两种处理方式：直接比较 `page_`，或增加私有方法 `auto IsEndImpl() const -> bool;` 供两者共用。

### 移动构造的必要性

`Begin()` 按值返回 `INDEXITERATOR_TYPE`。`return INDEXITERATOR_TYPE(bpm, page, idx);` 构造纯右值，C++17 起保证不发生拷贝或移动，无需额外处理。但若出现

```cpp
auto it = tree.Begin();
INDEXITERATOR_TYPE tmp = std::move(it);   // 或放入容器
```

则需要移动构造。禁止拷贝并允许移动，是唯一不会导致页面被重复释放的组合。

## 4 Begin / Begin(key) / End

### 4.1 三个方法当前均为占位实现

`src/storage/index/b_plus_tree.cpp` 现状：

```cpp
auto BPLUSTREE_TYPE::IsEmpty() const -> bool { return true; }        // 恒 true
auto BPLUSTREE_TYPE::GetRootPageId() -> page_id_t { return 0; }      // 恒 0
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }
```

由此有四条结论：

- 不应使用 `IsEmpty()` 判断空树。该方法当前恒返回 `true`，在成员二完成之前，`if (IsEmpty()) return End();` 将始终返回结束态，且不产生任何错误提示，表现为扫描结果为空，容易被误判为插入未生效。
- 不应使用 `GetRootPageId()`。该方法当前恒返回 `0`，即 header 页页号，不是根页页号。
- 判空应直接检查成员 `root_page_id_ == INVALID_PAGE_ID`，构造函数即按此初始化（`b_plus_tree.cpp` 第 15 行）。
- 三个占位实现返回默认构造的迭代器，没有抛出异常。调用方取得 `IsEnd()` 为真的对象，形式与空树一致，实际表示功能尚未实现。这一情形比抛出异常更难发现。

### 4.2 Begin()

```cpp
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE {
  if (root_page_id_ == INVALID_PAGE_ID) {
    return INDEXITERATOR_TYPE();
  }
  Page *page = FindLeafPage(KeyType(), /*leftmost=*/true);
  if (page == nullptr) {
    return INDEXITERATOR_TYPE();
  }
  return INDEXITERATOR_TYPE(buffer_pool_manager_, page, 0);
}
```

已知限制：若最左叶子页已被删除至 `GetSize() == 0` 而根页尚未收缩，以 `index = 0` 构造迭代器会越界。两种处理方式：`Begin()` 沿 `next` 链寻找第一个非空叶子页，或由成员三保证删除后立即收缩。此项需与成员三确认，不能假定由对方承担。

### 4.3 Begin(key)

```text
1. root_page_id_ == INVALID_PAGE_ID → 返回结束态
2. page = FindLeafPage(key)
3. 在 page 内用 comparator_ 查找第一个 >= key 的下标（读取 GetItem(i).first，不直接比较整数）
4. 若下标等于 page->GetSize()：取 GetNextPageId()，有效则进入新页并置 index = 0，无效则返回结束态
5. 否则以 (page, 下标) 构造迭代器
```

第 4 步对应 key 位于两页之间的情况，容易遗漏：key 大于左页全部键且小于右页最小键时，定位落在左页，下标恰好等于页内元素数。

## 5 需要覆盖的用例

| 场景 | 构造方式 | 预期 |
| --- | --- | --- |
| 空树 | 不插入，直接调用 `Begin()` | `Begin() == End()` |
| 单页 | `leaf_max_size = 3`，插入 3 条 | 从 `Begin()` 走到 `End()` 共 3 次，顺序升序 |
| 多页跨页 | `leaf_max_size = 3`，插入 10 条 | 遍历结果与 `std::set` 的顺序一致，次数为 10 |
| `Begin(key)` 落在两页之间 | 插入 1、2、3 与 10、11、12，调用 `Begin(5)` | 首项为 10，即右页第 0 项 |
| `Begin(key)` 大于最大值 | 插入 1 至 9，调用 `Begin(100)` | 结果等于 `End()` |

另建议补充两项：`Begin(key)` 小于最小值（应得第 0 项）、`Begin(key)` 恰好等于某个已存在的键（应得该项，不是下一项）。

预期值不应从被测实现取得。应使用 `std::set<int>` 的遍历结果，或以常量形式固定预期序列，否则等同于用实现验证实现。

## 6 与成员五的锁约定

- 迭代器只持有 pin，不获取锁。锁由成员五在 Fetch 与 Unpin 的外层统一处理。
- 页内读取顺序为 Fetch → RLatch → 读取 → RUnlatch → Unpin，须先解锁再 Unpin。顺序颠倒会留下页面仍持锁却被换出的窗口。
- 跨页（`operator++`）时，先 Fetch 新页并获取读锁，再解锁并 Unpin 旧页。两个动作之间不应出现两页均不受保护的空窗，否则并发删除可能在该时刻合并当前页。
- 上述顺序须与成员三删除合并时获取兄弟页的顺序一致，同为自右向左或同为自左向右。方向相反将形成循环等待。
- 请成员五确认第三条与 latch crabbing 的实现顺序是否兼容。

## 7 测试与构建

| 项 | 值 |
| --- | --- |
| 源文件 | `test/p2_quality/p2_quality_iterator_test.cpp` |
| CMake 目标名 | `p2_quality_iterator_test` |
| 建议超时 | 120 s |
| 依赖 | 无外部数据文件，全部使用小容量内存树 |
| 容量参数 | `leaf_max_size = 3`、`internal_max_size = 3`，十数个 key 即可产生跨页 |

上述目标须在测试文件存在后重新执行 `cmake -S . -B build/p2-asan` 才会生成。

## 8 待确认事项

- [ ] 成员一（吴佳忆）：新增 `GetItem(int) const -> const MappingType &`；`Init()` 中将 `next_page_id_` 置为 `INVALID_PAGE_ID`
- [ ] 成员二（梁怡莲）：`FindLeafPage(key, leftmost)` 返回已 pin 的 `Page *`，调用方负责 Unpin，空树返回 `nullptr`
- [ ] 成员三（肖子瑞）：删除后叶子页被删空时是否保证当场收缩，此项决定 `Begin()` 是否需跳过空叶子页
- [ ] 成员五（何佳绮）：跨页时先获取新页读锁、再释放旧页的顺序是否与 latch crabbing 兼容
- [ ] 组长：本文件合并至 `docs/p2/contracts/`，并将 `INTERFACES.md` 第 51 行状态改为「已定稿 v1」
