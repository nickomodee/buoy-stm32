import serial
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from scipy.spatial.transform import Rotation as R
import threading
from matplotlib.animation import FuncAnimation

SERIAL_PORT = 'COM5'
BAUD_RATE = 115200

latest_quaternion = [0, 0, 0, 1]
lock = threading.Lock()

def serial_reader():
    global latest_quaternion
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
        print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
    except serial.SerialException as e:
        print(f"Error opening serial port {SERIAL_PORT}: {e}")
        return

    while True:
        try:
            data = ser.read(16) # the 4 floats with the quaternion data in order: `i`, `j`, `k`, `real`
            if len(data) != 16:
                continue

            quat_i, quat_j, quat_k, quat_real = np.frombuffer(data, dtype='<f4') # convert raw bytes in little endian format back to floats
            # quat_i, quat_j = -quat_i, quat_j  # flip x and y axes to match visualation to the imu
            quaternion = [quat_i, quat_j, quat_k, quat_real]

            print(f"{quat_i},{quat_j},{quat_k},{quat_real}")

            with lock:
                latest_quaternion = quaternion

        except Exception as e:
            print(f"Error reading serial: {e}")
        except KeyboardInterrupt:
            break

    ser.close()

def quaternion_to_rotation_matrix(quaternion):
    return R.from_quat(quaternion).as_matrix()

# a slab like the imu
box_width = 2.0
box_depth = 1.0
box_height = 0.2

vertices = np.array([[-box_width / 2, -box_depth/2, -box_height/2],
                     [box_width / 2, -box_depth/2, -box_height/2],
                     [box_width/2, box_depth/2, -box_height/2],
                     [-box_width/2, box_depth/2, -box_height/2],
                     [-box_width/2, -box_depth/2, box_height/2],
                     [box_width/2, -box_depth/2, box_height/2],
                     [box_width/2, box_depth/2, box_height/2],
                     [-box_width/2,  box_depth/2, box_height/2]])

# source: https://blender.stackexchange.com/questions/45582/vertices-generated-by-script-not-placed-correctly
faces = [
    [0, 1, 2, 3],  # top
    [4, 5, 6, 7],  # bottom
    [0, 3, 7, 4],  # right
    [5, 6, 2, 1],  # left
    [3, 2, 6, 7],  # front
    [0, 1, 5, 4]   # back
]

fig = plt.figure(figsize=(15, 15))
ax = fig.add_subplot(111, projection='3d') # same as p3.Axes3d(fig)

ax.set_xlim([-3, 3])
ax.set_ylim([-3, 3])
ax.set_zlim([-3, 3])
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.set_title('IMU Visualiser')

face_colors = ['red', 'green', 'blue', 'purple', 'cyan', 'yellow']
poly3d = [[vertices[vert] for vert in face] for face in faces]
collection = Poly3DCollection(poly3d, facecolors=face_colors, linewidths=1, edgecolors='black', alpha=0.25)
ax.add_collection3d(collection)

def init_plot():
    return collection

# update plot with latest quaternion to simulate an animation
def update(frame):
    global latest_quaternion
    with lock:
        quaternion = latest_quaternion

    R = quaternion_to_rotation_matrix(quaternion)
    rotated_vertices = vertices.dot(R.T)
    new_poly3d = [[rotated_vertices[vert] for vert in face] for face in faces]
    collection.set_verts(new_poly3d)
    return collection

serial_thread = threading.Thread(target=serial_reader, daemon=True)
serial_thread.start()

ani = FuncAnimation(fig, update, init_func=init_plot, interval=0.01, blit=False) # start animation

plt.show()