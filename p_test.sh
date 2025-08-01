#!/bin/bash

# 微码映射引擎批量调试测试脚本

echo "=== 微码映射引擎批量调试测试 ==="

HARDWARE_DIR="/Users/jiahui/code/huawei-microcode-mapping/data/hardware_files"
SOFTWARE_FILE="/Users/jiahui/code/huawei-microcode-mapping/data/simple_680.txt"
OUTPUT_BASE_DIR="/Users/jiahui/code/huawei-microcode-mapping/tmp/build/tools/microcode-mapping-test/output"
MICROCODE="/Users/jiahui/code/huawei-microcode-mapping/tmp/build/tools/microcode-mapping-test/microcode-mapping-test"

echo "硬件AST文件夹: $HARDWARE_DIR"
echo "软件AST文件: $SOFTWARE_FILE"
echo "输出基础目录: $OUTPUT_BASE_DIR"

# 检查文件夹是否存在
if [ ! -d "$HARDWARE_DIR" ]; then
    echo "错误: 硬件AST文件夹不存在: $HARDWARE_DIR"
    exit 1
fi

if [ ! -f "$SOFTWARE_FILE" ]; then
    echo "错误: 软件AST文件不存在: $SOFTWARE_FILE"
    exit 1
fi

# 检查可执行文件是否存在
if [ ! -f "$MICROCODE" ]; then
    echo "错误: 微码映射引擎可执行文件不存在: $MICROCODE"
    exit 1
fi

# 创建输出基础目录
mkdir -p "$OUTPUT_BASE_DIR"

# 计数器
total_files=0
success_count=0
error_count=0
no_match_count=0

# 创建日志文件
LOG_FILE="$OUTPUT_BASE_DIR/microcode_test_result.txt"
> $LOG_FILE

echo ""
echo "开始批量处理硬件文件..."

# 遍历硬件文件夹中的所有文件
for hardware_file in "$HARDWARE_DIR"/*; do
    # 检查是否是文件
    if [ -f "$hardware_file" ]; then
        total_files=$((total_files + 1))
        
        # 获取文件名（不含路径）
        filename=$(basename "$hardware_file")
        # 获取文件名（不含扩展名）
        basename_no_ext="${filename%.*}"
        
        # 创建按照bash_test.sh格式的目录结构
        # single_test1, single_test2, ...
        single_test_dir="$OUTPUT_BASE_DIR/single_test$total_files"
        hard_acc_dir="$single_test_dir/hard_acc"
        
        echo ""
        echo "处理文件 $total_files: $filename"
        echo "输出目录: $single_test_dir"
        
        # 创建目录结构
        mkdir -p "$single_test_dir"
        mkdir -p "$hard_acc_dir"
        
        # 复制硬件文件到hard_acc.txt
        cp "$hardware_file" "$single_test_dir/hard_acc.txt"
        
        # 运行程序，使用临时输出目录
        temp_output_dir="$OUTPUT_BASE_DIR/temp_$basename_no_ext"
        mkdir -p "$temp_output_dir"
        
        if $MICROCODE "$hardware_file" "$SOFTWARE_FILE" "$temp_output_dir"; then
            echo "✓ 成功处理: $filename"
            success_count=$((success_count + 1))
            
            # 检查是否有匹配结果
            if [ -d "$temp_output_dir" ] && [ "$(ls -A "$temp_output_dir" 2>/dev/null)" ]; then
                # 有匹配结果，将结果移动到hard_acc目录
                for match_file in "$temp_output_dir"/*.txt; do
                    if [ -f "$match_file" ]; then
                        # 获取匹配文件名
                        match_filename=$(basename "$match_file")
                        # 移动到hard_acc目录
                        mv "$match_file" "$hard_acc_dir/"
                        echo "  ✓ 匹配结果: $match_filename"
                    fi
                done
                
                # 清理临时目录
                rm -rf "$temp_output_dir"
            else
                # 没有匹配结果，删除创建的目录结构
                echo "  ✗ 无匹配结果，删除目录结构"
                rm -rf "$single_test_dir"
                no_match_count=$((no_match_count + 1))
            fi
        else
            echo "✗ 处理失败: $filename"
            error_count=$((error_count + 1))
            # 清理临时目录和创建的目录结构
            rm -rf "$temp_output_dir"
            rm -rf "$single_test_dir"
        fi
    fi
done

echo ""
echo "=== 批量处理完成 ==="
echo "总文件数: $total_files"
echo "成功处理: $success_count"
echo "处理失败: $error_count"
echo "无匹配结果: $no_match_count"

# 写入日志文件
echo "总文件数: $total_files" >> $LOG_FILE
echo "成功处理: $success_count" >> $LOG_FILE
echo "处理失败: $error_count" >> $LOG_FILE
echo "无匹配结果: $no_match_count" >> $LOG_FILE
echo "--------------------------------" >> $LOG_FILE

# 如果有失败的文件，列出详细信息
if [ $error_count -gt 0 ]; then
    echo ""
    echo "失败的文件列表:"
    for hardware_file in "$HARDWARE_DIR"/*; do
        if [ -f "$hardware_file" ]; then
            filename=$(basename "$hardware_file")
            basename_no_ext="${filename%.*}"
            temp_output_dir="$OUTPUT_BASE_DIR/temp_$basename_no_ext"
            
            # 检查临时输出目录是否为空或不存在
            if [ ! -d "$temp_output_dir" ] || [ -z "$(ls -A "$temp_output_dir" 2>/dev/null)" ]; then
                echo "  - $filename"
            fi
        fi
    done
fi

echo ""
echo "所有输出文件保存在: $OUTPUT_BASE_DIR"
echo "日志文件: $LOG_FILE" 