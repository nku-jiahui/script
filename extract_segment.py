#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
提取segment0到segment1之间的内容
"""

import sys
import os

def extract_segment(input_file, output_file):
    """
    从输入文件中提取segment0到segment1之间的内容
    
    Args:
        input_file (str): 输入文件路径
        output_file (str): 输出文件路径
    """
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        start_line = -1
        end_line = -1
        
        # 查找segment0和segment1的行号
        for i, line in enumerate(lines):
            if 'segment 0' in line:
                start_line = i
            elif 'segment 1' in line:
                end_line = i
                break
        
        if start_line == -1:
            print(f"错误：在文件 {input_file} 中未找到 'segment 0'")
            return False
        
        if end_line == -1:
            print(f"错误：在文件 {input_file} 中未找到 'segment 1'")
            return False
        
        if start_line >= end_line:
            print(f"错误：segment 0 在 segment 1 之后或同一行")
            return False
        
        # 提取segment0和segment1之间的内容（不包含这两行）
        extracted_content = lines[start_line + 1:end_line]
        
        # 写入输出文件
        with open(output_file, 'w', encoding='utf-8') as f:
            f.writelines(extracted_content)
        
        print(f"成功提取内容：")
        print(f"  输入文件：{input_file}")
        print(f"  输出文件：{output_file}")
        print(f"  提取行数：{len(extracted_content)} 行")
        print(f"  行号范围：{start_line + 2} 到 {end_line}")
        
        return True
        
    except FileNotFoundError:
        print(f"错误：文件 {input_file} 不存在")
        return False
    except Exception as e:
        print(f"错误：{e}")
        return False

def main():
    """主函数"""
    if len(sys.argv) != 3:
        print("用法：python extract_segment.py <输入文件> <输出文件>")
        print("示例：python extract_segment.py input.txt output.txt")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    if not os.path.exists(input_file):
        print(f"错误：输入文件 {input_file} 不存在")
        sys.exit(1)
    
    success = extract_segment(input_file, output_file)
    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main() 