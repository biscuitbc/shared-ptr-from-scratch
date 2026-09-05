# 接口契约

本文定义本实验的评分契约。教学结构参考 [CS106L Assignment 7](https://github.com/cs106l/cs106l-assignments/tree/main/assignment7)，
共享所有权语义参考 C++ 工作草案的 [shared_ptr 定义](https://eel.is/c++draft/util.smartptr.shared)。
我们只要求下述子集；不要求实现工作草案的所有成员或最新语言特性。

## 范围和前置条件

- 使用 C++20；命名空间 `cs106l`；类型 `shared_ptr<T>`；别名 `element_type = T`。
- `T` 是非数组对象类型，可以是 `const U`，可以不可复制、不可移动、不可默认构造，也可过度对齐。
- 声明递归节点时允许 `T` 尚未完整。执行对象构造或 `delete T*` 的位置必须具有完整类型。
- 数组、引用、`void`、函数类型不支持，必须在实例化时拒绝。使用标量 `new/delete`，不可用 `delete[]` 管理对象。
- 只实现相同 `T` 的复制/移动；不要求跨类型转换、别名构造、自定义删除器、分配器、弱引用、`enable_shared_from_this`、比较或线程安全。
- 裸指针必须为 `nullptr`，或来自单对象 `new` 且适合 `delete T*`。不得传入栈对象、数组、已释放的地址，或已被其他所有权组管理的非空地址。
- 不支持依靠构造时动态类型信息进行多态删除。不要通过无虚析构的基类接管派生类；本实验的裸指针构造参数是 `T*`，不是标准库的 `Y*` 模板。
- 被管理对象的析构不得抛异常。调用解引用和箭头运算符时存储指针必须非空。
- 引用计数使用普通整数；只在单线程使用；计数不会超出 `long` 可表示范围。

以下行为违反前置条件，不属于测试对象：`p.reset(p.get())`、用 `p.get()` 新建独立所有权组、空指针解引用、对同一对象重复 `delete`。
框架不得用这些未定义行为来“验证”正确实现，也不得手动析构一个之后仍会自动析构的局部句柄。

## 所有权状态与空指针

控制块是共享的动态存储，至少包含引用计数；具体布局由学生决定。
没有控制块的状态为“空所有权”。有控制块时，每个所有者各贡献 1 次计数。
本实验没有别名构造，所以同一控制块对应的存储指针相同。

| 表达式（独立执行） | `get()` | `use_count()` | `bool` |
| --- | --- | ---: | --- |
| `shared_ptr<int>{}` | `nullptr` | 0 | false |
| `shared_ptr<int>{nullptr}` | `nullptr` | 0 | false |
| `shared_ptr<int>{new int(7)}` | 对象地址 | 1 | true |
| `shared_ptr<int>{static_cast<int*>(nullptr)}` | `nullptr` | 1 | false |

这个区别来自 [构造函数的后置条件](https://eel.is/c++draft/util.smartptr.shared.const)：
接管有类型的指针会创建所有权组，即使其地址为空。字面量 `nullptr` 走 `std::nullptr_t` 重载。
因此析构、复制和计数判断必须依据所有权状态，不能仅判断 `get()`。
默认/字面量空构造、复制、移动、观察器、`reset()` 和 `swap` 不得动态分配内存。
裸指针构造必须动态创建控制块；对象和控制块分开分配。

## 公开接口

保留 `shared_ptr.h` 中的函数签名、`explicit`、`const`、`noexcept` 和已提供的类型限制。
可增加私有字段和私有辅助函数，不增加完成作业所不需要的公开 API。

| 接口 | 必须满足的行为 |
| --- | --- |
| `shared_ptr() noexcept` | 默认空状态，不分配 |
| `shared_ptr(std::nullptr_t) noexcept` | 默认空状态，支持从 `nullptr` 隐式构造 |
| `explicit shared_ptr(T*)` | 接管指针并创建计数为 1 的控制块，失败时删除输入指针并传播异常 |
| `~shared_ptr() noexcept` | 释放本句柄所有权；计数归零时删除对象和控制块 |
| `shared_ptr(const shared_ptr&) noexcept` | 共享源的所有权，非空组计数加一；空源产生空句柄 |
| `operator=(const shared_ptr&) noexcept` | 取得源所有权的一份并释放旧所有权，返回 `*this` |
| `shared_ptr(shared_ptr&&) noexcept` | 转移状态，不增加计数，源恢复默认空状态 |
| `operator=(shared_ptr&&) noexcept` | 转移源状态并释放旧所有权，返回 `*this`；自移动保留原值 |
| `T* get() const noexcept` | 返回存储指针 |
| `T& operator*() const noexcept` | 返回对象引用，要求非空 |
| `T* operator->() const noexcept` | 返回对象指针，要求非空 |
| `long use_count() const noexcept` | 返回共享计数，无控制块返回 0 |
| `explicit operator bool() const noexcept` | 返回 `get() != nullptr`，不能隐式转成 `bool` 或整数 |
| `void reset() noexcept` | 释放本句柄所有权，恢复默认空状态 |
| `void reset(T*)` | 等价于 `shared_ptr(ptr).swap(*this)`，提供强异常保证 |
| `void swap(shared_ptr&) noexcept` | 交换对象指针和控制块状态，不改变任何组的计数 |
| 非成员 `swap` | 已提供；通过成员 `swap` 支持 ADL |
| `make_shared<T>(Args&&...)` | 完美转发，值初始化 `T`，返回计数为 1 的句柄 |

删除的 `reset(std::nullptr_t)` 重载已提供：清空用 `p.reset()`，不要写 `p.reset(nullptr)`。
标准库的模板 `reset(Y*)` 也不能从字面量 `nullptr` 推导 `Y`。
`p.reset(static_cast<T*>(nullptr))` 则是接管一个有类型的空指针，成功后计数为 1。

按照 [观察器接口](https://eel.is/c++draft/util.smartptr.shared.obs)，`const shared_ptr<int>` 仍允许修改所指整数。
`shared_ptr<const int>` 才限制对所指对象的修改，不要求从 `shared_ptr<int>` 转换构造它。

## 赋值的边界

自复制和自移动必须保持地址、计数及对象值不变。两个不同的句柄即使属于同一组，也不等于自赋值。
若原来 `a`、`b` 共同持有一个计数为 2 的对象，则 `a = std::move(b)` 后 `a.use_count() == 1`、`b.use_count() == 0`。

必须支持源句柄是目标所拥有对象的成员，例如 `p = p->next` 或 `p = std::move(p->next)`。
释放旧 `p` 可能销毁 `other` 所在节点，因此要在释放旧对象前取得安全的新状态，之后不再读取已销毁的成员。
可考虑先构造一个局部临时句柄，再交换状态。不要为了绕过这个问题禁止这些合法操作。

复制/移动/观察器/交换应为 O(1)。释放最后一个对象时，其自身析构的时间不计在句柄操作的常数开销内；链表析构可能递归释放多个节点。

## 异常安全

`shared_ptr<T>(raw)` 中，若控制块分配失败，构造函数必须 `delete raw` 并重抛。
失败的构造不会调用该句柄的析构函数，所以不能依靠析构补救。

`p.reset(raw)` 中，若新控制块分配失败，新对象必须被删除，旧 `p` 及旧组其他句柄的地址、值和计数必须保持不变。
这是 [reset 语义](https://eel.is/c++draft/util.smartptr.shared.mod) 在异常路径上的要求。

`make_shared` 要支持以下路径：对象分配失败、对象构造抛异常、对象成功而控制块分配失败。
都必须传播原异常并回收已经取得的资源。
本实验要求简单的“两次分配”实现，不要求也不允许为通过此实验改变为单块合并分配；这使控制块分配失败能够被确定地注入测试。
标准库的 `make_shared` 常采用合并分配优化，本实验不宣称相同的分配行为。

## 链表应用

`create_list(const std::vector<T>& values, shared_ptr<ListNode<T>> tail = nullptr)`：

- `T` 可拷贝构造；`tail` 是无环、有限链表。
- 仅为 `values` 中的元素创建新节点，保持原顺序，不修改输入 vector。
- 最后一个新节点的 `next.get()` 必须恰好等于原 `tail.get()`，共享原控制块。
- 输入为空时原样返回 tail；空 tail 加非空输入产生普通链表。
- 不修改 tail 中的值或链接，不复制其节点，不创建环。
- 若构造前缀时发生异常，已经建好的前缀必须释放，调用者原 tail 及 values 保持不变。

链表函数只允许使用本实验智能指针管理节点。`print_list` 与节点定义已提供，不需修改。
测试分别检查节点值、节点身份、共享计数、生命周期和异常清理。
