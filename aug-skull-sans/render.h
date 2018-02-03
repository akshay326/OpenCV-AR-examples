//
// Created by akshay on 29/1/18.
//
#include<opencv2/core.hpp>

#ifndef IRON_HELMET_RENDER_H
#define IRON_HELMET_RENDER_H

bool loadOBJ(
        const char * path,
        std::vector<cv::Point3d> & out_vertices,
        std::vector<cv::Point3d> & out_normals
);

void indexVBO(
        std::vector<cv::Point3d> & in_vertices,
        std::vector<cv::Point3d> & in_normals,

        std::vector<unsigned short> & out_indices,
        std::vector<cv::Point3d> & out_vertices,
        std::vector<cv::Point3d> & out_normals,
        float min_distance
);


#endif //IRON_HELMET_RENDER_H
