#####################################################################################################
# @file         main.py
# @brief        水稻病虫害监测系统 - K230部署版 v6
# @description  基于实际测试数据优化的阈值和分类逻辑
#####################################################################################################

import time, os
from media.sensor import *
from media.display import *
from media.media import *

# ==================== 颜色阈值 (LAB色彩空间) ====================
# 基于实际K230测试数据调优
# 格式: (L_lo, L_hi, A_lo, A_hi, B_lo, B_hi)
THRESHOLDS = {
    # 健康叶片: 绿色，A通道为负（偏绿）
    "green_leaf": {
        "lab": (15, 85, -128, -15, -10, 35),
        "name_cn": "健康",
        "name_en": "Healthy",
        "color": (0, 255, 0, 255),
    },
    # 稻瘟病: 褐色病斑，A偏正（偏红），B偏正（偏黄）
    # 放宽A_lo让更多褐色被检测到
    "brown_spot": {
        "lab": (10, 80, -5, 80, 10, 80),
        "name_cn": "稻瘟病",
        "name_en": "Rice Blast",
        "color": (255, 50, 50, 255),
    },
    # 白叶枯病: 黄白色，L亮度高，B偏正（偏黄）
    # 收窄范围避免把偏黄的正常背景误检
    "yellow_white": {
        "lab": (45, 100, -10, 15, 20, 80),
        "name_cn": "白叶枯病",
        "name_en": "Blight",
        "color": (255, 255, 0, 255),
    },
}

# 过滤参数
PIXELS_THRESHOLD = 3000       # 最小像素数
MAX_BLOBS_TO_SHOW = 3          # 最多显示框数
DISEASE_RATIO_MIN = 0.08      # 病害面积占比阈值（8%以上才判定为病害）
DISEASE_AREA_MIN = 5000        # 病害绝对面积最小值

# ==================== 分类逻辑 ====================
def classify(blob_areas):
    """
    基于面积比例的综合分类逻辑
    规则:
      1. 绿色面积 > 85% 且 病害面积 < 3000 => 健康
      2. 褐色面积 > 黄白面积 且 褐色占比 > 8% => 稻瘟病
      3. 黄白面积 > 褐色面积 且 黄白占比 > 8% => 白叶枯病
      4. 其他 => 健康
    """
    green = blob_areas.get("green_leaf", 0)
    brown = blob_areas.get("brown_spot", 0)
    yellow = blob_areas.get("yellow_white", 0)
    total = green + brown + yellow + 1

    green_pct = green / total
    brown_pct = brown / total
    yellow_pct = yellow / total

    # 规则1: 绿色占绝对优势 => 健康
    if green_pct > 0.85 or (brown < DISEASE_AREA_MIN and yellow < DISEASE_AREA_MIN):
        return "green_leaf", green_pct

    # 规则2: 褐色 > 黄白，且褐色占比足够 => 稻瘟病
    if brown > yellow and brown > DISEASE_AREA_MIN and brown_pct > DISEASE_RATIO_MIN:
        return "brown_spot", brown_pct

    # 规则3: 黄白 > 褐色，且黄白占比足够 => 白叶枯病
    if yellow > brown and yellow > DISEASE_AREA_MIN and yellow_pct > DISEASE_RATIO_MIN:
        return "yellow_white", yellow_pct

    # 规则4: 有病害但不够明显 => 健康
    return "green_leaf", green_pct

# ==================== 绘制结果 ====================
def draw_result(img, name_cn, name_en, color, confidence, areas):
    """绘制识别结果"""
    # 顶部状态栏
    img.draw_rectangle(0, 0, 640, 70, color=(0, 0, 0, 255), thickness=-1)
    label = "{} {} {:.0f}%".format(name_cn, name_en, confidence)
    img.draw_string_advanced(15, 15, 28, label, color=color)

    # 底部信息栏
    img.draw_rectangle(0, 410, 640, 70, color=(0, 0, 0, 255), thickness=-1)

    g = areas.get("green_leaf", 0)
    b = areas.get("brown_spot", 0)
    y = areas.get("yellow_white", 0)
    total = g + b + y + 1
    info_text = "G:{}({:.0f}%) B:{}({:.0f}%) Y:{}({:.0f}%)".format(
        g, g/total*100, b, b/total*100, y, y/total*100
    )
    img.draw_string_advanced(15, 415, 14, info_text, color=(200, 200, 200, 255))

    if name_cn != "健康":
        img.draw_string_advanced(15, 440, 20, "! 检测到病虫害 !", color=(255, 0, 0, 255))
    else:
        img.draw_string_advanced(15, 440, 20, "状态: 生长良好", color=(0, 255, 0, 255))

    return img

# ==================== 获取最大的几个blob ====================
def get_top_blobs(blobs, max_count=3):
    if not blobs:
        return []
    sorted_blobs = sorted(blobs, key=lambda b: b[4], reverse=True)
    return sorted_blobs[:max_count]

# ==================== 主程序 ====================
def main():
    print("=" * 50)
    print("水稻病虫害监测系统 v6")
    print("基于实际数据优化")
    print("=" * 50)

    sensor = None
    try:
        sensor = Sensor(width=1280, height=960)
        sensor.reset()
        sensor.set_framesize(Sensor.VGA)
        sensor.set_pixformat(Sensor.RGB565)

        Display.init(Display.ST7701, width=640, height=480, fps=90, to_ide=True)
        MediaManager.init()
        sensor.run()

        print("摄像头初始化完成")

        clock = time.clock()
        update_counter = 0

        # 稳定显示
        cur_name_cn = "健康"
        cur_name_en = "Healthy"
        cur_color = (0, 255, 0, 255)
        cur_confidence = 95
        cur_areas = {"green_leaf": 0, "brown_spot": 0, "yellow_white": 0}

        while True:
            os.exitpoint()
            clock.tick()

            img = sensor.snapshot()

            # 每15帧识别一次
            update_counter += 1
            if update_counter >= 15:
                update_counter = 0

                # 查找各颜色blob
                blob_areas = {}
                for key, info in THRESHOLDS.items():
                    blobs = img.find_blobs(
                        [info["lab"]],
                        pixels_threshold=PIXELS_THRESHOLD
                    )
                    total_area = 0
                    for blob in blobs:
                        total_area += blob[4]
                    blob_areas[key] = total_area

                # 分类
                detected_key, disease_pct = classify(blob_areas)
                info = THRESHOLDS[detected_key]
                cur_name_cn = info["name_cn"]
                cur_name_en = info["name_en"]
                cur_color = info["color"]

                if detected_key == "green_leaf":
                    cur_confidence = min(99, int(80 + disease_pct * 100))
                else:
                    cur_confidence = min(99, int(70 + disease_pct * 200))

                cur_areas = blob_areas

                g = blob_areas.get("green_leaf", 0)
                b = blob_areas.get("brown_spot", 0)
                y = blob_areas.get("yellow_white", 0)
                print("G:{} B:{} Y:{} => {}".format(g, b, y, cur_name_cn))

            # 绘制结果
            img = draw_result(img, cur_name_cn, cur_name_en, cur_color, cur_confidence, cur_areas)

            # 只绘制当前识别病害的框（健康时不画框）
            if cur_name_cn != "健康":
                for key, info in THRESHOLDS.items():
                    if info["name_cn"] != cur_name_cn:
                        continue
                    blobs = img.find_blobs(
                        [info["lab"]],
                        pixels_threshold=PIXELS_THRESHOLD
                    )
                    top_blobs = get_top_blobs(blobs, MAX_BLOBS_TO_SHOW)
                    for blob in top_blobs:
                        img.draw_rectangle(
                            blob[0], blob[1], blob[2], blob[3],
                            color=info["color"], thickness=3
                        )

            # FPS
            fps_text = "FPS:{}".format(int(clock.fps()))
            img.draw_string_advanced(520, 80, 18, fps_text, color=(255, 255, 0, 255))

            Display.show_image(img)

    except KeyboardInterrupt as e:
        print("user stop: ", e)
    except BaseException as e:
        print("Exception:", e)
    finally:
        if isinstance(sensor, Sensor):
            sensor.stop()
        Display.deinit()
        os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
        time.sleep_ms(100)
        MediaManager.deinit()
        print("系统已关闭")

if __name__ == "__main__":
    main()
