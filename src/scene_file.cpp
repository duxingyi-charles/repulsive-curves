#include "scene_file.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace LWS {

    std::string getDirectoryFromPath(std::string str) {
        using namespace std;
        vector<string> parts;
        splitString(str, parts, '/');

        int nParts = parts.size();
        if (nParts == 1) return "./";
        
        string path = "";

        for (int i = 0; i < nParts - 1; i++) {
            path = path + parts[i] + "/";
        }

        return path;
    }

    template<typename T>
    bool vectorContains(std::vector<T> &vec, T t) {
        for (size_t i = 0; i < vec.size(); i++) {
            if (vec[i] == t) return true;
        }
        return false;
    }

    void processLine(SceneData &data, std::string dir_root, std::vector<std::string> &parts) {
        using namespace std;
        string key = parts[0];

        if (key == "curve") {
            if (parts.size() != 2) {
                std::cerr << "Incorrect arguments to curve" << std::endl;
                exit(1);
            }
            data.curve_filename = dir_root + parts[1];
        }

        // ========== Potentials ==========

        else if (key == "repel_curve") {
            if (parts.size() <= 1) {
                data.tpe_alpha = 3;
                data.tpe_beta = 6;
            }
            else if (parts.size() == 3) {
                data.tpe_alpha = stod(parts[1]);
                data.tpe_beta = stod(parts[2]);
            }
            else if (parts.size() == 4) {
                data.tpe_alpha = stod(parts[1]);
                data.tpe_beta = stod(parts[2]);
                data.tpe_weight = stod(parts[3]);
            }
            else {
                std::cerr << "Incorrect arguments to repel_curve" << std::endl;
                exit(1);
            }
        }

        else if (key == "repel_surface") {
            if (parts.size() < 2 || parts.size() > 3) {
                std::cerr << "Incorrect arguments to repel_surface" << std::endl;
                exit(1);
            }
            else if (parts.size() == 2) {
                data.obstacles.push_back(ObstacleData{dir_root + parts[1], 1});
            }
            else if (parts.size() == 3) {
                data.obstacles.push_back(ObstacleData{dir_root + parts[1], stod(parts[2])});
            }
        }

        else if (key == "optimize_length") {
            if (parts.size() == 1) {
                data.extraPotentials.push_back(PotentialData{PotentialType::Length, 1, ""});
            }
            else if (parts.size() == 2) {
                data.extraPotentials.push_back(PotentialData{PotentialType::Length, stod(parts[1]), ""});
            }
            else {
                std::cerr << "Incorrect arguments to optimize_length" << std::endl;
                exit(1);
            }
        }

        else if (key == "optimize_area") {
            if (parts.size() == 1) {
                data.extraPotentials.push_back(PotentialData{PotentialType::Area, 1, ""});
            }
            else if (parts.size() == 2) {
                data.extraPotentials.push_back(PotentialData{PotentialType::Area, stod(parts[1]), ""});
            }
            else {
                std::cerr << "Incorrect arguments to optimize_area" << std::endl;
                exit(1);
            }
        }

        else if (key == "optimize_field") {
            if (parts.size() == 2) {
                data.extraPotentials.push_back(PotentialData{PotentialType::VectorField, 1, parts[1]});
            }
            else if (parts.size() == 3) {
                data.extraPotentials.push_back(PotentialData{PotentialType::VectorField, stod(parts[2]), parts[1]});
            }
            else {
                std::cerr << "Incorrect arguments to optimize_field" << std::endl;
                exit(1);
            }
        }


        // ========== Constraints ==========
        else if (key == "fix_barycenter") {
            if (parts.size() != 1) {
                std::cerr << "fix_barycenter does not take any arguments" << std::endl;
                exit(1);
            }
            data.constraints.push_back(ConstraintType::Barycenter);
        }

        else if (key == "fix_length") {
            std::cerr << "Total length constraint is not implemented" << std::endl;
            exit(1);
        }

        else if (key == "fix_edgelengths") {
            if (parts.size() == 1) {
                data.constraints.push_back(ConstraintType::EdgeLengths);
            }
            else if (parts.size() == 2) {
                data.constraints.push_back(ConstraintType::EdgeLengths);
                std::cerr << "Edge length scale is not implemented" << std::endl;
                exit(1);
            }
            else {
                std::cerr << "Incorrect arguments to fix_edgelengths" << std::endl;
                exit(1);
            }
        }

        else if (key == "fix_vertex") {
            if (parts.size() == 2) {
                if (!vectorContains(data.constraints, ConstraintType::Pins)) {
                    data.constraints.push_back(ConstraintType::Pins);
                }
                data.pinnedVertices.push_back(stoi(parts[1]));
            }
            else {
                std::cerr << "Incorrect arguments to fix_vertex" << std::endl;
                exit(1);
            }
        }

        else if (key == "fix_tangent") {
            if (parts.size() == 2) {
                if (!vectorContains(data.constraints, ConstraintType::TangentPins)) {
                    data.constraints.push_back(ConstraintType::TangentPins);
                }
                data.pinnedTangents.push_back(stoi(parts[1]));
            }
            else {
                std::cerr << "Incorrect arguments to fix_tangent" << std::endl;
                exit(1);
            }
        }
        else if (key == "constraint_surface") {
            std::cerr << "Constraint surfaces are not implemented" << std::endl;
            exit(1);
        }
        else if (key == "constrain_vertex") {
            std::cerr << "Constraint surfaces are not implemented" << std::endl;
            exit(1);
        }
        else if (key == "constrain_all") {
            std::cerr << "Constraint surfaces are not implemented" << std::endl;
            exit(1);
        }
        
        else if (key == "#") {
            return;
        }

        else {
            std::cerr << "Unrecognized keyword " << key << std::endl;
        }
    }

    SceneData ParseSceneFile(std::string filename) {
        using namespace std;
        SceneData sceneData;
        string directory = getDirectoryFromPath(filename);
        std::cout << "Base directory of scene file: " << directory << std::endl;

        ifstream inFile;
        inFile.open(filename);

        if (!inFile) {
            cerr << "Could not open file " << filename << endl;
            exit(1);
        }
    
        std::vector<std::string> parts;
        for (std::string line; std::getline(inFile, line ); ) {
            if (line == "" || line == "\n") continue;
            parts.clear();
            splitString(line, parts, ' ');
            processLine(sceneData, directory, parts);
        }

        inFile.close();
        return sceneData;
    }

}