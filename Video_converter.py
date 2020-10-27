import cv2
import numpy as np
import os


def bin_generator(path=0, load_fps=40, threshold=128, filename="binfile", res=(128, 64)):
    video = cv2.VideoCapture(path)
    f = open(filename + ".bin", "w")
    f.write("")
    f.close()
    while video.isOpened():
        ret, frame = video.read()
        if frame is None:
            frame_num = video.get(cv2.CAP_PROP_FRAME_COUNT)
            file_size = os.path.getsize(filename + ".bin")
            print("Finished! {} frames, {} bytes".format(frame_num, file_size))
            break
        resized_frame = cv2.resize(frame, (res[0], res[1]), interpolation=cv2.INTER_AREA)
        greyscale = cv2.cvtColor(resized_frame, cv2.COLOR_RGB2GRAY)
        thresh, bw = cv2.threshold(greyscale, threshold, 255, cv2.THRESH_BINARY)
        save_frame(np.array(bw), filename)
        cv2.imshow('bw', bw)

        if cv2.waitKey(load_fps) & 0xFF == ord('q'):
            break
    video.release()
    cv2.destroyAllWindows()


def save_frame(frame, filename):
    bin_file = open(filename + ".bin", "ab+")
    for page in range(int(frame.shape[0] / 8)):
        for col in range(frame.shape[1]):
            bin_array = ""
            for pix in range(8):
                if frame[page * 8 + pix][col] == 255:
                    bin_array += "1"
                else:
                    bin_array += "0"
            bin_file.write(int(bin_array, 2).to_bytes(1, byteorder='big'))
    bin_file.close()


bin_generator(path='sample.mp4', load_fps=1, threshold=80)