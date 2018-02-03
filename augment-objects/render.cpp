//
// Created by akshay on 29/1/18.
//

#include <map>
#include "render.h"

bool loadOBJ(const char *path, std::vector<cv::Point3d> &out_vertices, std::vector<cv::Point3d> &out_normals){
        printf("Loading OBJ file %s...\n", path);

        std::vector<unsigned int> vertexIndices, normalIndices;
        std::vector<cv::Point3d> temp_vertices;
        std::vector<cv::Point3d> temp_normals;


        FILE * file = fopen(path, "r");
        if( file == nullptr ){
            printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
            getchar();
            return false;
        }

        while( true ){

            char lineHeader[128];

            // read the first word of the line
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
                break; // EOF = End Of File. Quit the loop.

            // else : parse lineHeader

            if ( strcmp( lineHeader, "v" ) == 0 ) {
                cv::Point3d vertex;
                fscanf(file, "%lf %lf %lf\n", &vertex.x, &vertex.y, &vertex.z);
                temp_vertices.push_back(vertex);
            }else if ( strcmp( lineHeader, "vn" ) == 0 ){
                cv::Point3d normal;
                fscanf(file, "%lf %lf %lf\n", &normal.x, &normal.y, &normal.z );
                temp_normals.push_back(normal);
            }else if ( strcmp( lineHeader, "f" ) == 0 ){
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
                if (matches != 6){
                    printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                    fclose(file);
                    return false;
                }
                vertexIndices.push_back(temp_vertices.size() + vertexIndex[0]);
                vertexIndices.push_back(temp_vertices.size() + vertexIndex[1]);
                vertexIndices.push_back(temp_vertices.size() + vertexIndex[2]);
                normalIndices.push_back(temp_normals.size() + normalIndex[0]);
                normalIndices.push_back(temp_normals.size() + normalIndex[1]);
                normalIndices.push_back(temp_normals.size() + normalIndex[2]);
            }else{
                // Probably a comment, eat up the rest of the line
                char stupidBuffer[1000];
                fgets(stupidBuffer, 1000, file);
            }

        }

        // For each vertex of each triangle
        for( unsigned int i=0; i<vertexIndices.size(); i++ ){

            // Get the indices of its attributes
            unsigned int vertexIndex = vertexIndices[i];
            unsigned int normalIndex = normalIndices[i];

            // Get the attributes thanks to the index
            cv::Point3d vertex = temp_vertices[ vertexIndex]; //vertexIndex-1 ];
            cv::Point3d normal = temp_normals[ normalIndex]; //normalIndex-1 ];

            // Put the attributes in buffers
            out_vertices.push_back(vertex);
            out_normals .push_back(normal);

        }

        fclose(file);

        return true;
    }

// Searches through all already-exported vertices for a similar one.
// Similar = same position + same normal
bool getSimilarVertexIndex(
        cv::Point3d& in_vertex,
        cv::Point3d& in_normal,
        std::vector<cv::Point3d> & out_vertices,
        std::vector<cv::Point3d> & out_normals,
        unsigned short & result,
        const float min_distance
){
    // Lame linear search
    for ( unsigned short i=0; i<out_vertices.size(); i++ ){
        if (
                fabs( in_vertex.x - out_vertices[i].x )<min_distance &&
                fabs( in_vertex.y - out_vertices[i].y )<min_distance &&
                fabs( in_vertex.z - out_vertices[i].z )<min_distance &&
                fabs( in_normal.x - out_normals [i].x )<min_distance &&
                fabs( in_normal.y - out_normals [i].y )<min_distance &&
                fabs( in_normal.z - out_normals [i].z )<min_distance
                ){
            result = i;
            return true;
        }
    }
    // No other vertex could be used instead.
    // Looks like we'll have to add it to the VBO.
    return false;
}

/**
 * @brief Reduces the normal, vertices size by removing very closely placed ones
 * @param in_vertices
 * @param in_normals
 * @param out_indices
 * @param out_vertices
 * @param out_normals
 * @param min_distance Min Distance to be maintained b/t points
 */
void indexVBO(std::vector<cv::Point3d> &in_vertices, std::vector<cv::Point3d> &in_normals,
              std::vector<unsigned short> &out_indices, std::vector<cv::Point3d> &out_vertices,
              std::vector<cv::Point3d> &out_normals,const float min_distance) {

    // For each input vertex
    for ( unsigned int i=0; i<in_vertices.size(); i++ ){

        // Try to find a similar vertex in out_XXXX
        unsigned short index;
        bool found = getSimilarVertexIndex(in_vertices[i], in_normals[i], out_vertices, out_normals, index, min_distance);

        if ( found ) // A similar vertex is already in the VBO, use it instead !
            out_indices.push_back( index );
        else{ // If not, it needs to be added in the output data.
            out_vertices.push_back( in_vertices[i]);
            out_normals .push_back( in_normals[i]);
            out_indices .push_back(out_vertices.size() - 1);
        }
    }
}
