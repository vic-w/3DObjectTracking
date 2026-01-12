# 读取一个obj文件,以及mtl和png贴图文件
# 给定相机的内参
# 给定物体到世界坐标系的变换body2world
# 渲染出物体在相机里的拍摄到的图片

import numpy as np
import trimesh
import pyrender
import imageio

def read_first_pose_rbot_dataset(path: str) -> np.ndarray:
    """
    读取 RBOT 数据集的首帧位姿，返回 4x4 仿射矩阵（齐次坐标）。
    文件格式：第一行跳过；之后依次为 3x3 旋转（tab 分隔）和 3 个平移（mm，需要转米）。
    """
    pose = np.eye(4, dtype=np.float32)
    with open(path, "rb") as f:
        # 跳过第一行
        _ = f.readline()
        # 读取 3x3 旋转
        for i in range(3):
            for j in range(3):
                token = f.read().split(b'\t', 1)[0]  # 读取到下一个 '\t'
                # 上面读法较繁琐，改用文本模式更简单；为稳妥改用下面实现：
                pass

    # 更简洁的文本实现
    with open(path, "r", encoding="utf-8") as f:
        f.readline()  # 跳过首行
        # 读取 3x3 旋转
        for i in range(3):
            for j in range(3):
                token = f.read(1)  # 占位，不实际使用
        # 重新按行解析
        f.seek(0)
        f.readline()
        tokens = []
        # 旋转 9 个值 + 平移 3 个值，共 12 个，以制表符/换行分隔
        while len(tokens) < 12:
            line = f.readline()
            if not line:
                break
            # 按制表符拆分，末尾可能是换行
            parts = [p for p in line.strip().split('\t') if p != ""]
            tokens.extend(parts)

        # 前 9 个是旋转
        R = np.array([float(x) for x in tokens[:9]], dtype=np.float32).reshape(3, 3)
        t = np.array([float(x) for x in tokens[9:12]], dtype=np.float32) * 0.001  # mm→m
        pose[:3, :3] = R
        pose[:3, 3] = t
    return pose


obj_path = "/home/vic/code/3DObjectTracking/data/RBOT_dataset/ape/ape.obj"


K = np.array([[650.048, 0,       324.328],
              [0,       647.183, 257.323],
              [0,       0,       1]])
fx, fy, cx, cy = K[0,0], K[1,1], K[0,2], K[1,2]
img_width, img_height = 640, 512  # 按需设置分辨率

body2world = read_first_pose_rbot_dataset("/home/vic/code/3DObjectTracking/data/RBOT_dataset/poses_first.txt")
world2camera = np.eye(4, dtype=np.float32)
body2camera = world2camera @ body2world
print("body2camera:\n", body2camera)
cv2gl = np.array([[1, 0, 0, 0],
                  [0,-1, 0, 0],
                  [0, 0,-1, 0],
                  [0, 0, 0, 1]], dtype=np.float32)
body2camera_gl = cv2gl @ body2camera

mesh = trimesh.load(obj_path, force='mesh', skip_materials=False)
scale_factor = 0.001
mesh.apply_scale(scale_factor)

mesh_tr = pyrender.Mesh.from_trimesh(mesh, smooth=True)

# 构建场景（bg_color 用 0~1 浮点）
scene = pyrender.Scene(bg_color=[1, 1, 1, 1.0], ambient_light=[0.8, 0.8, 0.8])
scene.add(mesh_tr, pose=body2camera_gl)

# 添加相机（pinhole）
camera_left = pyrender.IntrinsicsCamera(fx=fx, fy=fy, cx=cx, cy=cy, znear=0.01, zfar=10.0)
camera_left_node = scene.add(camera_left, pose=np.eye(4))
camera_right = pyrender.IntrinsicsCamera(fx=fx, fy=fy, cx=cx, cy=cy, znear=0.01, zfar=10.0)
pose_r = np.eye(4)
pose_r[0, 3] = 0.1  # 右相机平移 10cm
camera_right_node = scene.add(camera_right, pose=pose_r)

# 添加简单光源
light = pyrender.DirectionalLight(color=np.ones(3), intensity=3.0)
scene.add(light, pose=np.eye(4))

r = pyrender.OffscreenRenderer(viewport_width=img_width, viewport_height=img_height)

scene.main_camera_node = camera_left_node  # <--- 关键：指定主相机节点
image_l, depth_l = r.render(scene)
imageio.imwrite('image_l.png', image_l)

# --- 渲染 Camera B 的视角 ---
scene.main_camera_node = camera_right_node  # <--- 关键：切换主相机节点
image_r, depth_r = r.render(scene)
imageio.imwrite('image_r.png', image_r)

# # 离屏渲染
# r = pyrender.OffscreenRenderer(viewport_width=img_width, viewport_height=img_height)
# color, depth = r.render(scene)
# r.delete()

# # 保存或显示
# import imageio
# imageio.imwrite('render_ape.png', color)
# print('Saved to render_ape.png')