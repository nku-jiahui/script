#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
提取txt文件中callActionName后面的accName
用法: python extract_acc_names.py <txt文件路径>
"""

import re
import sys
from pathlib import Path
from collections import Counter

def extract_acc_names(file_path):
    """从文件中提取callActionName后面的accName"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"错误: 无法读取文件 {file_path}: {e}")
        return []
    
    # 正则表达式匹配 callActionName accName[参数] 格式
    # 匹配 callActionName 后面跟着的 accName，参数部分可选
    pattern = r'callActionName\s+(\w+)\s*\[.*?\]'
    
    # 查找所有匹配项
    matches = re.findall(pattern, content)
    
    return matches

def analyze_acc_names(acc_names):
    """分析accName的统计信息"""
    if not acc_names:
        return
    
    # 统计每个accName出现的次数
    counter = Counter(acc_names)
    
    print(f"找到 {len(acc_names)} 个callActionName调用:")
    print("-" * 50)
    
    # 按出现次数排序
    for acc_name, count in counter.most_common():
        print(f"{acc_name:<20} : {count:>3} 次")
    
    print("-" * 50)
    print(f"不同的accName数量: {len(counter)}")
    print(f"总调用次数: {len(acc_names)}")
    
    # 显示所有唯一的accName（按字母顺序）
    print(f"\n所有唯一的accName (按字母顺序):")
    print("-" * 30)
    for acc_name in sorted(counter.keys()):
        print(f"  {acc_name}")

def main():
    if len(sys.argv) != 2:
        print("用法: python extract_acc_names.py <txt文件路径>")
        print("示例: python extract_acc_names.py input.txt")
        sys.exit(1)
    
    file_path = sys.argv[1]
    
    if not Path(file_path).exists():
        print(f"错误: 文件不存在: {file_path}")
        sys.exit(1)
    
    print(f"正在分析文件: {file_path}")
    print("=" * 60)
    
    # 提取accName
    acc_names = extract_acc_names(file_path)
    
    if not acc_names:
        print("未找到任何callActionName调用")
        return
    
    # 分析结果
    analyze_acc_names(acc_names)
    
    # 保存结果到文件
    output_file = f"{Path(file_path).stem}_acc_names.txt"
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(f"从文件 {file_path} 提取的accName列表:\n")
            f.write("=" * 50 + "\n\n")
            
            # 写入统计信息
            counter = Counter(acc_names)
            f.write("统计信息:\n")
            f.write("-" * 30 + "\n")
            for acc_name, count in counter.most_common():
                f.write(f"{acc_name}: {count} 次\n")
            
            f.write(f"\n总调用次数: {len(acc_names)}\n")
            f.write(f"不同accName数量: {len(counter)}\n\n")
            
            # 写入所有调用（按出现顺序）
            f.write("所有调用 (按出现顺序):\n")
            f.write("-" * 30 + "\n")
            for i, acc_name in enumerate(acc_names, 1):
                f.write(f"{i:>3}. {acc_name}\n")
        
        print(f"\n结果已保存到: {output_file}")
        
    except Exception as e:
        print(f"警告: 无法保存结果文件: {e}")

if __name__ == "__main__":
    main() 