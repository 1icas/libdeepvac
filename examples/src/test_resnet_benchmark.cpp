/*
 * Copyright (c) 2020 Gemfield <gemfield@civilnet.cn>
 * This file is part of libdeepvac, licensed under the GPLv3 (the "License")
 * You may not use this file except in compliance with the License.
 */
#include "syszux_cls_resnet.h"
#include "syszux_img2tensor.h"
#include "gemfield.h"
#include "deepvac.h"
#include <chrono>
#include <c10/cuda/CUDAStream.h>
#include <ATen/cuda/CUDAContext.h>
#include <ATen/ATen.h>
#include <ATen/Parallel.h>
#include "syszux_imagenet_classes.h"
#include <stdlib.h>

using namespace deepvac;
int main(int argc, const char* argv[]) {
    if (argc != 4) {
        GEMFIELD_E("usage: deepvac <device> <model_path> <img_path>");
        return -1;
    }

    //at::init_num_threads();
    //at::set_num_threads(16);
    //at::set_num_interop_threads(16);
    std::cout<<"userEnabledCuDNN: "<<at::globalContext().userEnabledCuDNN()<<std::endl;
    std::cout<<"userEnabledMkldnn: "<<at::globalContext().userEnabledMkldnn()<<std::endl;
    std::cout<<"benchmarkCuDNN: "<<at::globalContext().benchmarkCuDNN()<<std::endl;
    std::cout<<"deterministicCuDNN: "<<at::globalContext().deterministicCuDNN()<<std::endl;
    std::cout<<"gemfield thread num: "<<at::get_num_threads()<<" | "<<at::get_num_interop_threads()<<std::endl;
    std::string device = argv[1];
    std::string model_path = argv[2];
    std::string img_path = argv[3];
    
    std::cout << "img_path : " << img_path << std::endl;
    SyszuxClsResnet cls_resnet(model_path, device);
 
    cls_resnet.set({224, 224});

    auto mat_opt = gemfield_org::img2CvMat(img_path);
    if(!mat_opt){
        throw std::runtime_error("illegal image detected!");
        return 1;
    }
    auto mat_out = mat_opt.value();
    for (int i=0; i<1000; i++) {
        auto stream = at::cuda::getCurrentCUDAStream();
        AT_CUDA_CHECK(cudaStreamSynchronize(stream));
        auto start = std::chrono::system_clock::now();

        auto resnet_out_opt = cls_resnet.process(mat_out);

        stream = at::cuda::getCurrentCUDAStream();
        AT_CUDA_CHECK(cudaStreamSynchronize(stream));

        std::chrono::duration<double> model_loading_duration = std::chrono::system_clock::now() - start;
        std::string msg = gemfield_org::format("whole process time: %f", model_loading_duration.count());
        std::cout << msg << std::endl<< std::endl;

        if(!resnet_out_opt){
            throw std::runtime_error("return empty error!");
        }
        
        auto resnet_out = resnet_out_opt.value();
        std::cout << "Index: " << resnet_out.first << std::endl;
        std::cout << "Classes: " << gemfield_org::imagenet_classes[resnet_out.first] << std::endl;
        std::cout << "Probability: " << resnet_out.second << std::endl;
    }
    return 0;
}