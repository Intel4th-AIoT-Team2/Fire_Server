import socket
import cv2
import numpy as np

# 서버 설정
server_address = "서버IP"  # 서버의 IP로 변경
server_port = 5000

# 서버 연결
def connect_to_server():
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((server_address, server_port))
        return client_socket
    except Exception as e:
        print(f"서버 연결 실패 : {e}")
        return None

client_socket = connect_to_server()

if client_socket:
    try:
        # 응답 받기
        response = client_socket.recv(1024).decode("utf-8")
        print(f"{response}\n")

        # webcam
        cap = cv2.VideoCapture(0)
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]

        while True:
            try:
                ret, frame = cap.read()
                if not ret:
                    break

                # 이미지를 JPEG 형식으로 인코딩
                result, frame = cv2.imencode('.jpg', frame, encode_param)
                data = np.array(frame)
                byteData = data.tobytes()

                # 데이터의 크기를 먼저 전송 -> 데이터 전송
                client_socket.sendall((str(len(byteData))).encode().ljust(16) + byteData)
            except BrokenPipeError:
                print("[Error] 서버와 연결이 끊어졌습니다. 연결을 재시도합니다.")
                client_socket.close()
                client_socket = connect_to_server()
                if not client_socket:
                    break
            except Exception as e:
                print(f"[연결 종료] {e}")
                break
    finally:
        if client_socket is not None:
                client_socket.close()
	
    cap.release()
else:
    print("서버에 연결할 수 없어 프로그램을 종료합니다.")
