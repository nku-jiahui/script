#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
统计文件夹中所有txt文件的行数
用法: python count_lines.py <文件夹路径>
"""

import os
import sys
from pathlib import Path

def count_lines_in_file(file_path):
    """统计单个文件的行数"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return len(f.readlines())
    except Exception as e:
        print(f"错误: 无法读取文件 {file_path}: {e}")
        return -1

def count_lines_in_directory(directory_path):
    """统计目录中所有txt文件的行数"""
    directory = Path(directory_path)
    
    if not directory.exists():
        print(f"错误: 目录不存在: {directory_path}")
        return
    
    if not directory.is_dir():
        print(f"错误: 不是目录: {directory_path}")
        return
    
    # 获取所有txt文件
    txt_files = list(directory.glob("*.txt"))
    
    if not txt_files:
        print(f"在目录 {directory_path} 中没有找到txt文件")
        return
    
    print(f"在目录 {directory_path} 中找到 {len(txt_files)} 个txt文件:")
    print("-" * 80)
    
    total_lines = 0
    file_count = 0
    line_counts = []  # 存储所有文件的行数
    
    # 按文件名排序
    txt_files.sort()
    
    for file_path in txt_files:
        line_count = count_lines_in_file(file_path)
        if line_count >= 0:
            print(f"{file_path.name:<30} {line_count:>6} 行")
            total_lines += line_count
            file_count += 1
            line_counts.append(line_count)
    
    print("-" * 80)
    print(f"总计: {file_count} 个文件, {total_lines} 行")
    
    # 统计行数分布
    if line_counts:
        print("\n行数分布统计:")
        print("-" * 40)
        
        # 定义行数区间
        ranges = [
            (0, 10, "0-10行"),
            (11, 20, "11-20行"),
            (21, 50, "21-50行"),
            (51, 100, "51-100行"),
            (101, 200, "101-200行"),
            (201, 500, "201-500行"),
            (501, 1000, "501-1000行"),
            (1001, float('inf'), "1000行以上")
        ]
        
        for min_lines, max_lines, range_name in ranges:
            if max_lines == float('inf'):
                count = sum(1 for lines in line_counts if lines > min_lines)
            else:
                count = sum(1 for lines in line_counts if min_lines <= lines <= max_lines)
            
            if count > 0:
                percentage = (count / len(line_counts)) * 100
                print(f"{range_name:<12}: {count:>3} 个文件 ({percentage:>5.1f}%)")
        
        # 显示统计信息
        if line_counts:
            min_lines = min(line_counts)
            max_lines = max(line_counts)
            avg_lines = sum(line_counts) / len(line_counts)
            print("-" * 40)
            print(f"最少行数: {min_lines} 行")
            print(f"最多行数: {max_lines} 行")
            print(f"平均行数: {avg_lines:.1f} 行")

def main():
    if len(sys.argv) != 2:
        print("用法: python count_lines.py <文件夹路径>")
        print("示例: python count_lines.py /path/to/folder")
        sys.exit(1)
    
    directory_path = sys.argv[1]
    count_lines_in_directory(directory_path)

if __name__ == "__main__":
    main() 