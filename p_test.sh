#!/bin/bash

# 微码映射引擎批量调试测试脚本
# 功能：批量处理硬件文件，生成匹配结果
# 输出格式：
# output/
# ├── single_test1/
# │   ├── hardware1.txt          (硬件AST文件)
# │   └── hardware1/             (硬件文件名文件夹，包含匹配结果)
# │       ├── soft_fuc1.txt
# │       ├── soft_fuc2.txt
# │       └── ...
# ├── single_test2/
# │   ├── hardware2.txt
# │   └── hardware2/
# └── ...

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
        
        # 创建single_test目录
        single_test_dir="$OUTPUT_BASE_DIR/single_test$total_files"
        
        echo ""
        echo "处理文件 $total_files: $filename"
        echo "输出目录: $single_test_dir"
        
        # 创建single_test目录
        mkdir -p "$single_test_dir"
        
        # 复制硬件文件到single_test目录
        cp "$hardware_file" "$single_test_dir/$filename"
        
        # 运行微码映射程序
        if $MICROCODE "$hardware_file" "$SOFTWARE_FILE" "$single_test_dir"; then
            echo "✓ 成功处理: $filename"
            success_count=$((success_count + 1))
        elif [ $? -eq 2 ]; then
            # 退出码2表示没有匹配结果
            echo "✗ 无匹配结果: $filename"
            no_match_count=$((no_match_count + 1))
            # 删除single_test目录（因为没有匹配结果）
            rm -rf "$single_test_dir"
        else
            echo "✗ 处理失败: $filename"
            error_count=$((error_count + 1))
            # 删除single_test目录（因为处理失败）
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

echo ""
echo "所有输出文件保存在: $OUTPUT_BASE_DIR" 