# 📞 Phonebook System - 高性能电话簿管理系统

[![C](https://img.shields.io/badge/C-11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

一个基于**双哈希索引**和**链表**的高性能电话簿管理系统，支持 O(1) 复杂度的姓名/电话查询。

## ✨ 核心特性

### 数据结构
- **单向链表**：插入/删除操作 O(1) 时间复杂度
- **双哈希索引**：姓名哈希表 + 电话哈希表，精确查询 O(1)
- **快速排序**：支持按姓名/电话/地址排序，时间复杂度 O(n log n)

### 功能列表
- ✅ 增删改查（双哈希自动同步）
- ✅ 分页显示（N/P/G 翻页，可配置每页数量）
- ✅ 模糊查询（姓名/电话/地址关键词匹配）
- ✅ CSV 批量导入/导出（支持 Excel 打开）
- ✅ 多维度排序（姓名/电话/地址）

### 系统特性
- 📁 二进制文件持久化 + 完整性校验
- 📝 INI 配置文件解析
- 📋 日志审计系统（操作记录带时间戳）
- 🛠️ Makefile 自动化构建
- ⚠️ 完整错误处理（内存分配失败回滚、errno 级别定位）

## 🚀 快速开始

### 编译运行
```bash
# 编译
gcc phonebook.c -o phonebook -fexec-charset=GBK

# 运行
./phonebook