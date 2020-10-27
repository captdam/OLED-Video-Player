import cv2
import numpy as np
import time
def bin_generator(path=0, load_fps=40, threshold=128):
    video = cv2.VideoCapture(path)
    while video.isOpened():

        ret, frame = video.read()
        Framecount = int(video.get(cv2.CAP_PROP_FRAME_COUNT))
        print(Framecount)
        time.sleep(10)
        resized_frame = cv2.resize(frame, (128, 64), interpolation=cv2.INTER_AREA)
        greyscale = cv2.cvtColor(resized_frame, cv2.COLOR_RGB2GRAY)
        thresh, bw = cv2.threshold(greyscale, threshold, 255, cv2.THRESH_BINARY)
        save_frame(np.array(bw))

        print("end")
        cv2.imshow('bw', bw)
        if cv2.waitKey(load_fps) & 0xFF == ord('q'):
            break
    video.release()
    # cv2.destroyWindow(bw)


def save_frame(frame):
    print(frame.shape)
    bin_file = open("binfile.txt", "a")
    for page in range(int(frame.shape[0] / 8)):
        for col in range(frame.shape[1]):
            for pix in range(8):
                if frame[page * 8 + pix][col] == 255:
                    bin_file.write('1')
                else:
                    bin_file.write('0')
    bin_file.close()


def pictest(path):
    pic = cv2.imread(path, 0)
    thresh, bw = cv2.threshold(pic, 128, 255, cv2.THRESH_BINARY)
    cv2.imshow("bw", bw)
    data = np.array(bw)
    print(data.shape)


#pictest("sample.jpg")
bin_generator(path='sample.mp4', load_fps=50, threshold=80)