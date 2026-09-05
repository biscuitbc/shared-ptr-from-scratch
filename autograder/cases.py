"""Canonical case inventory and hints. / 唯一用例清单及提示。"""

CASES = {
    "basics": {
        "empty_default": "S1/S3: default-empty state / 默认空状态",
        "literal_null": "S1/S3: nullptr overload has count 0 / 字面量空构造计数为 0",
        "typed_null": "S2/S3: typed null owns a block / 有类型的空指针拥有控制块",
        "raw_observers": "S1–S3: identity and observers / 对象身份和观察器",
        "const_handle": "S1: const handle still permits mutable T / const 句柄仍允许修改 T",
        "const_pointee": "S1: support const T / 支持 const T",
        "destruction": "S3: destroy exactly once, including unwinding / 正常及异常路径恰好析构一次",
    },
    "ownership": {
        "copy_constructor": "S4: share object and counter / 共享对象和计数",
        "copy_empty": "S4: empty copy stays empty / 复制空句柄仍为空",
        "copy_typed_null": "S4: count owners even when get() is null / 空存储指针也须统计所有者",
        "copy_assignment_releases": "S4: release the old object / 释放旧对象",
        "copy_assignment_shared_target": "S4: preserve other old owners / 保留旧组的其他所有者",
        "copy_assignment_empty": "S4: assignment to/from empty / 空状态双向赋值",
        "copy_self_and_same_group": "S4: distinguish self from same group / 区分自身和同组",
        "move_constructor": "S5: transfer and empty source / 转移并清空源",
        "move_assignment": "S5: release old share, preserve new peers / 释放旧份额，保留新组同伴",
        "move_empty_and_null": "S5: empty and typed-null moves / 空所有权及有类型空指针的移动",
        "move_self_and_same_group": "S5: preserve self-move, empty distinct source / 自移动保留，异源移动清空源",
        "nested_assignment": "S4/S5: acquire source before destroying its container / 先取得源，再销毁其所属对象",
        "differential_sequences": "S1–S6: 10,000 deterministic operations vs std / 与标准库对照一万次确定性操作",
    },
    "modifiers": {
        "reset_empty": "S6: repeated empty reset / 重复清空",
        "reset_last": "S6: last-owner reset / 最后一个所有者清空",
        "reset_shared": "S6: preserve surviving owners / 保留其他所有者",
        "reset_replace": "S6: replace empty, unique, shared states / 替换空、独占、共享状态",
        "reset_typed_null": "S6: typed-null reset creates a new group / 有类型空指针重置建立新组",
        "swap_groups": "S6: swap complete states / 交换完整状态",
        "swap_empty_self_adl": "S6: empty, self and ADL swap / 空状态、自身和 ADL 交换",
    },
    "factory": {
        "value_init": "S7: value-initialize zero arguments / 零参数值初始化",
        "multi_args": "S7: multiple arguments, const and alignment / 多参数、const 和对齐",
        "forwarding": "S7: preserve argument value categories / 保留参数值类别",
        "move_only": "S7: move-only argument, immovable object / 仅可移动参数、不可移动对象",
        "throwing_constructor": "S7: propagate construction failure without leaks / 传播构造异常且不泄漏",
    },
    "application": {
        "empty": "S8: empty values and tail / 空值序列和空尾部",
        "order": "S8: preserve order and counts / 保持顺序及计数",
        "strings": "S8: generic value type / 泛型值类型",
        "identity": "S8: share the exact tail and its counter / 共享原尾部及控制块",
        "empty_with_tail": "S8: return existing tail for empty values / 空序列返回已有尾部",
        "shared_lifetime": "S8: release only unshared prefix / 只释放不再共享的前缀",
        "prefix_exception": "S8: clean partial prefixes on every failure / 各失败路径清理部分前缀",
        "repeated_sharing": "S8: repeated sharing and moderate length / 重复共享与适中长度链表",
    },
    "exceptions": {
        "raw_alloc_failure": "S2: delete raw object if block allocation fails / 控制块失败时删除裸对象",
        "typed_null_alloc_failure": "S2: even typed null allocates / 有类型空指针也分配控制块",
        "reset_alloc_failure": "S6: strong exception guarantee / 强异常保证",
        "reset_empty_alloc_failure": "S6: preserve empty or null-owned old state / 保留旧空状态或空指针所有权",
        "factory_object_alloc_failure": "S7: object allocation failure / 对象分配失败",
        "factory_control_alloc_failure": "S7: second allocation failure / 第二次分配失败",
        "no_alloc_operations": "S1/S3–S6: no allocations in noexcept operations / noexcept 操作不分配",
    },
    "interface": {
        "api_contract": "S1–S6: signatures, explicit, const, noexcept / 签名与类型约束",
        "multiple_translation_units": "S7: self-contained header and ODR / 头文件独立性及多翻译单元",
        "rejected_programs": "API: reject unsupported types and implicit conversions / 拒绝不支持类型及隐式转换",
    },
}

REJECTED_PROGRAMS = {
    "implicit_raw": "int* raw = nullptr; lab::shared_ptr<int> p = raw;",
    "implicit_bool": "lab::shared_ptr<int> p; bool b = p; (void)b;",
    "implicit_int": "lab::shared_ptr<int> p; int n = p; (void)n;",
    "implicit_pointer": "lab::shared_ptr<int> p; int* raw = p; (void)raw;",
    "mutate_const": "lab::shared_ptr<const int> p; *p = 5;",
    "unbounded_array": "lab::shared_ptr<int[]> p;",
    "bounded_array": "lab::shared_ptr<int[3]> p;",
    "void_type": "lab::shared_ptr<void> p;",
    "reference_type": "lab::shared_ptr<int&> p;",
    "function_type": "lab::shared_ptr<int()> p;",
    "reset_literal_null": "lab::shared_ptr<int> p; p.reset(nullptr);",
}

assert sum(map(len, CASES.values())) == 50
