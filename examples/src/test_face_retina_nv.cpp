/*
 * Copyright (c) 2020 Gemfield <gemfield@civilnet.cn>
 * This file is part of libdeepvac, licensed under the GPLv3 (the "License")
 * You may not use this file except in compliance with the License.
 */

#include <chrono>
#include "syszux_face_retina_nv.h"
#include "syszux_face_reg_nv.h"
#include "syszux_img2tensor.h"

using namespace deepvac;
int main(int argc, const char* argv[]) {
    if (argc != 3) {
        GEMFIELD_E("usage: deepvac <device> <img_path>");
        return -1;
    }

    std::string device = argv[1];
    std::string img_path = argv[2];
    SyszuxFaceRetinaNV face_detect("detect.trt", device);
    SyszuxFaceRegNV face_reg("reg.trt", device);

    auto start = std::chrono::system_clock::now(); 

    for(int i = 0; i < 155; ++i) {
        auto start1 = std::chrono::system_clock::now();
        std::string img_name = img_path + std::to_string(i*10) + ".jpg";
        auto mat_opt = gemfield_org::img2CvMat(img_name);

        if(!mat_opt){
            throw std::runtime_error("illegal image detected");
            return 1;
        }

        auto mat_out = mat_opt.value();
        auto detect_out_opt = face_detect.process(mat_out);
        std::chrono::duration<double> model_loading_duration_d = std::chrono::system_clock::now() - start1;
        std::string msg = gemfield_org::format("Model loading time: %f", model_loading_duration_d.count());
        std::cout << msg << std::endl;

        if(detect_out_opt){
            face_reg.process(detect_out_opt);
        }

        std::chrono::duration<double> model_loading_duration_d1 = std::chrono::system_clock::now() - start1;
        std::string msg1 = gemfield_org::format("Model loading time: %f", model_loading_duration_d1.count());
        std::cout << msg1 << std::endl;
    }

    std::chrono::duration<double> model_loading_duration = std::chrono::system_clock::now() - start;
    std::string msg = gemfield_org::format("Model loading time: %f", model_loading_duration.count());
    std::cout << msg << std::endl;
    return 0;
}
