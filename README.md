# board_detect

初始化：
move_target_detect mtd;
识别方法：
flag = mtd.loop_process(current_pic);
current_pic为当前视频帧，返回1表示保存报警图片，返回2返回报警消除图片，图片存在当前目录
返回最后存储的图片名称：
mtd.Return_saved_file();
