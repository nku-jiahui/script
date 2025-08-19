#!/bin/bash

# 支持跳跃segment编号的循环脚本，带超时控制和日志记录
REPO_DIR=$(cd $(dirname $BASH_SOURCE[0])/.. && pwd)

# 配置参数
SEGMENTS=(1 3 5 6)  # 在这里指定您需要的segment编号
TIMEOUT_HOURS=3     # 超时时间（小时）
LOG_FILE="$REPO_DIR/tmp/output/segments_execution.log"

# 创建输出目录
mkdir -p "$REPO_DIR/tmp/output"

# 清空日志文件
> "$LOG_FILE"

echo "=== Segment批量处理脚本 ===" | tee -a "$LOG_FILE"
echo "开始时间: $(date)" | tee -a "$LOG_FILE"
echo "超时设置: ${TIMEOUT_HOURS}小时" | tee -a "$LOG_FILE"
echo "日志文件: $LOG_FILE" | tee -a "$LOG_FILE"
echo "----------------------------------------" | tee -a "$LOG_FILE"

# 统计变量
TOTAL_SEGMENTS=${#SEGMENTS[@]}
COMPLETED_SEGMENTS=0
TIMEOUT_SEGMENTS=0
FAILED_SEGMENTS=0

echo "总共需要处理 $TOTAL_SEGMENTS 个segments: ${SEGMENTS[*]}" | tee -a "$LOG_FILE"
echo "" | tee -a "$LOG_FILE"

# 处理每个segment
for segment in "${SEGMENTS[@]}"; do
    echo "==========================================" | tee -a "$LOG_FILE"
    echo "开始处理 segment$segment ($(date))" | tee -a "$LOG_FILE"
    echo "==========================================" | tee -a "$LOG_FILE"
    
    # 检查segment文件夹是否存在
    SEGMENT_PATH="$REPO_DIR/data/bash_test/segment$segment"
    if [ ! -d "$SEGMENT_PATH" ]; then
        echo "错误: segment$segment 文件夹不存在: $SEGMENT_PATH" | tee -a "$LOG_FILE"
        echo "状态: 跳过（文件夹不存在）" | tee -a "$LOG_FILE"
        FAILED_SEGMENTS=$((FAILED_SEGMENTS + 1))
        echo "" | tee -a "$LOG_FILE"
        continue
    fi
    
    # 设置输出路径
    OUTPUT_PATH="$REPO_DIR/tmp/output/segment$segment"
    
    echo "输入路径: $SEGMENT_PATH" | tee -a "$LOG_FILE"
    echo "输出路径: $OUTPUT_PATH" | tee -a "$LOG_FILE"
    
    # 记录开始时间
    START_TIME=$(date +%s)
    
    # 使用timeout命令运行脚本，超时时间转换为秒
    TIMEOUT_SECONDS=$((TIMEOUT_HOURS * 3600))
    
    echo "设置超时时间: ${TIMEOUT_HOURS}小时 (${TIMEOUT_SECONDS}秒)" | tee -a "$LOG_FILE"
    
    # 运行脚本并捕获退出状态
    timeout $TIMEOUT_SECONDS bash -c "
        ./utils/bash_test.sh \\
            \"$REPO_DIR/data/reallocResult.txt\" \\
            \"$SEGMENT_PATH\" \\
            \"$OUTPUT_PATH\" \\
            \"$REPO_DIR/data/reflection.txt\"
    "
    
    EXIT_CODE=$?
    END_TIME=$(date +%s)
    DURATION=$((END_TIME - START_TIME))
    DURATION_HOURS=$((DURATION / 3600))
    DURATION_MINUTES=$(((DURATION % 3600) / 60))
    DURATION_SECONDS=$((DURATION % 60))
    
    echo "执行时间: ${DURATION_HOURS}小时${DURATION_MINUTES}分钟${DURATION_SECONDS}秒" | tee -a "$LOG_FILE"
    
    # 根据退出状态判断结果
    if [ $EXIT_CODE -eq 124 ]; then
        echo "状态: 超时终止 (超过${TIMEOUT_HOURS}小时)" | tee -a "$LOG_FILE"
        echo "segment$segment: 超时终止 - $(date)" >> "$LOG_FILE"
        TIMEOUT_SEGMENTS=$((TIMEOUT_SEGMENTS + 1))
    elif [ $EXIT_CODE -eq 0 ]; then
        echo "状态: 正常完成" | tee -a "$LOG_FILE"
        echo "segment$segment: 正常完成 - $(date)" >> "$LOG_FILE"
        COMPLETED_SEGMENTS=$((COMPLETED_SEGMENTS + 1))
    else
        echo "状态: 执行失败 (退出码: $EXIT_CODE)" | tee -a "$LOG_FILE"
        echo "segment$segment: 执行失败 (退出码: $EXIT_CODE) - $(date)" >> "$LOG_FILE"
        FAILED_SEGMENTS=$((FAILED_SEGMENTS + 1))
    fi
    
    echo "" | tee -a "$LOG_FILE"
done

# 生成最终报告
echo "==========================================" | tee -a "$LOG_FILE"
echo "=== 执行完成报告 ===" | tee -a "$LOG_FILE"
echo "结束时间: $(date)" | tee -a "$LOG_FILE"
echo "----------------------------------------" | tee -a "$LOG_FILE"
echo "总segment数: $TOTAL_SEGMENTS" | tee -a "$LOG_FILE"
echo "正常完成: $COMPLETED_SEGMENTS" | tee -a "$LOG_FILE"
echo "超时终止: $TIMEOUT_SEGMENTS" | tee -a "$LOG_FILE"
echo "执行失败: $FAILED_SEGMENTS" | tee -a "$LOG_FILE"
echo "----------------------------------------" | tee -a "$LOG_FILE"

# 详细状态报告
echo "" | tee -a "$LOG_FILE"
echo "=== 详细状态报告 ===" | tee -a "$LOG_FILE"

# 正常完成的segments
if [ $COMPLETED_SEGMENTS -gt 0 ]; then
    echo "✅ 正常完成的segments:" | tee -a "$LOG_FILE"
    for segment in "${SEGMENTS[@]}"; do
        if grep -q "segment$segment: 正常完成" "$LOG_FILE"; then
            echo "   - segment$segment" | tee -a "$LOG_FILE"
        fi
    done
    echo "" | tee -a "$LOG_FILE"
fi

# 超时的segments
if [ $TIMEOUT_SEGMENTS -gt 0 ]; then
    echo "⏰ 超时终止的segments:" | tee -a "$LOG_FILE"
    for segment in "${SEGMENTS[@]}"; do
        if grep -q "segment$segment: 超时终止" "$LOG_FILE"; then
            echo "   - segment$segment" | tee -a "$LOG_FILE"
        fi
    done
    echo "" | tee -a "$LOG_FILE"
fi

# 失败的segments
if [ $FAILED_SEGMENTS -gt 0 ]; then
    echo "❌ 执行失败的segments:" | tee -a "$LOG_FILE"
    for segment in "${SEGMENTS[@]}"; do
        if grep -q "segment$segment: 执行失败" "$LOG_FILE"; then
            echo "   - segment$segment" | tee -a "$LOG_FILE"
        fi
    done
    echo "" | tee -a "$LOG_FILE"
fi

echo "详细日志已保存到: $LOG_FILE" | tee -a "$LOG_FILE"
echo "==========================================" | tee -a "$LOG_FILE" 