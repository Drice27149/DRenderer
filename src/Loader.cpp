//#include <fstream>
//#include <iostream>
//#include <cassert>
//#include <cmath>
//#include <assimp/Importer.hpp>      // C++ importer interface
//#include <assimp/scene.h>           // Output data structure
//#include <assimp/postprocess.h> 
//
//#include "Loader.hpp"
//
//int isNumber(char c){
//	return (c >= '0' && c <= '9') || (c == '.') || (c =='-') || (c=='e');
//}
//
//float stringToFloat(std::string& s, int l, int r){
//	// judge 1.5e-002
//	int findE = -1;
//	for(int i = l; i <= r; i++) if(s[i] == 'e') findE = i;
//	if(findE != -1){
//		// adjust scale for small obj such as bunny.obj
//		return stringToFloat(s, l, findE-1) * std::pow(10.0, stringToFloat(s, findE+1, r)+2.0);
//	}
//	// judge 1.5
//	int neg = 0;
//	if(s[l] == '-') neg = 1, l++;
//	int dotPos = r+1;
//	for(int i = l; i <= r; i++) if(s[i] == '.') dotPos = i;
//	float left = 0.0f, right = 0.0f;
//	for(int i = l; i <= dotPos-1; i++){
//		left = left * 10.0 + (s[i] - '0');
//	}
//	for(int i = r; i >= dotPos+1; i--){
//		right = right * 0.1 + (s[i] - '0');
//	}
//	if(!neg) return left + right * 0.1;
//	else return -(left + right * 0.1);
//}
//
//int stringToInt(std::string& s, int l, int r){
//	int neg = 0;
//	if(s[l] == '-') neg = 1, l++;
//	int left = 0;
//	for(int i = l; i <= r; i++) left = left * 10 + s[i] - '0';
//	if(!neg) return left;
//	else return -left;
//}
//
//std::vector<float> parseFloat(std::string line){
//	std::vector<float> nums;
//	int ptr = 0;
//	while(ptr < line.size()){
//		if(!isNumber(line[ptr])) ptr++;
//		else{
//			int np = ptr;
//			while(np+1 < line.size() && isNumber(line[np+1])) np++;
//			nums.push_back(stringToFloat(line, ptr, np));
//			ptr = np + 1;
//		}
//	}
//	return nums;
//}
//
//std::vector<int> parseInt(std::string line){
//	std::vector<int> nums;
//	int ptr = 0;
//	while(ptr < line.size()){
//		if(!isNumber(line[ptr])) ptr++;
//		else{
//			int np = ptr;
//			while(np+1 < line.size() && isNumber(line[np+1])) np++;
//			nums.push_back(stringToInt(line, ptr, np));
//			ptr = np + 1;
//		}
//	}
//	return nums;
//}
//
//glm::vec3 getFaceNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c){
//    return glm::normalize(glm::cross(c - b, a - b));
//}
//
//void Loader::loadFile(std::string fileName){
//	vertices.clear();
//	stCoords.clear();
//	normals.clear();
//	vIndex.clear();
//	stIndex.clear();
//	nIndex.clear();
//	outStream.clear();
//
//	std::ifstream infile(fileName);
//	std::string line;
//	int lines = 0;
//	int fs = 0;
//
//	while(std::getline(infile, line)){
//		lines++;
//
//		if(line[0] == 'v' && line[1] != 't' && line[1] != 'n'){
//			// vertex x y z
//			std::vector<float> nums = parseFloat(line);
//			assert(nums.size() == 3);
//			vertices.push_back(glm::vec3(nums[0], nums[1], nums[2]));
//		}
//		else if(line[0] == 'v' && line[1] == 't'){
//			// stCoords x y
//			std::vector<float> nums = parseFloat(line);
//			assert(nums.size() == 2);
//			stCoords.push_back(glm::vec2(nums[0], nums[1]));
//		}
//		else if(line[0] == 'v' && line[1] == 'n'){
//			std::vector<float> nums = parseFloat(line);
//			assert(nums.size() == 3);
//			normals.push_back(glm::vec3(nums[0], nums[1], nums[2]));
//		}
//		else if(line[0] == 'f'){
//			fs++;
//			// face v/vt/vn
//			// 0 1 2, 3 4 5, 6 7 8
//			std::vector<int> nums = parseInt(line); 
//			if(nums.size()!=9) std::cout << line << "\n";
//            assert(nums.size() == 9);
//			for(int i = 0; i < 3; i++){
//				// minus one ?
//				vIndex.push_back(nums[i*3]-1);
//				stIndex.push_back(nums[i*3+1]-1);
//				nIndex.push_back(nums[i*3+2]-1);
//			}
//		}
//		else if(line[0] == '#' || line[0] == ' '){
//			// comment line
//		}
//		else{
//			/*std::cout << "invalid line: " << line << "\n";
//			int haveOtherToken = 1;
//			assert(haveOtherToken == 0);*/
//		}
//	}
//	int n = vIndex.size();
//	// generate out stream for data buffer
//	// vertice coordinates, texture coordinates, normal coordinates
//	// outStream used to call drawArray
//    for(int i = 0; i < n; i++){
//		for(int j = 0; j < 3; j++)
//			outStream.push_back(vertices[vIndex[i]][j]);
//		for(int j = 0; j < 2; j++)
//			outStream.push_back(stCoords[stIndex[i]][j]);
//		for(int j = 0; j < 3; j++)
//			outStream.push_back(normals[nIndex[i]][j]);
//    }
//	
//	getTangent();
//
//	printf("lines = %d, n = %d, faces = %d, outStream size = %d, t = %d, bt = %d\n", lines, n, fs, outStream.size(), tangents.size(), bitangents.size());
//}
//
//void Loader::getTangent()
//{
//	tangents.clear();
//	bitangents.clear();
//	int cnt = vIndex.size() / 3;
//	for(int i = 0; i < cnt; i++)
//	{
//		glm::vec3 v0 = vertices[vIndex[i*3]];
//		glm::vec3 v1 = vertices[vIndex[i*3+1]];
//		glm::vec3 v2 = vertices[vIndex[i*3+2]];
//		glm::vec2 uv0 = stCoords[stIndex[i*3]];
//		glm::vec2 uv1 = stCoords[stIndex[i*3+1]];
//		glm::vec2 uv2 = stCoords[stIndex[i*3+2]];
//	
//		glm::vec3 deltaPos1 = v1-v0;
//        glm::vec3 deltaPos2 = v2-v0;
//        glm::vec2 deltaUV1 = uv1-uv0;
//        glm::vec2 deltaUV2 = uv2-uv0;
//
//		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
//        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
//        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
//		for(int time = 0; time < 3; time++){
//			tangents.push_back(tangent);
//			bitangents.push_back(bitangent);
//		}
//	}
//}
//
//
//void Loader::AssimpLoadFile(std::string filename)
//{
//	Assimp::Importer importer;
//
//	const aiScene* scene = importer.ReadFile( filename,
//		aiProcess_CalcTangentSpace       |
//		aiProcess_Triangulate            |
//		aiProcess_JoinIdenticalVertices  |
//		aiProcess_SortByPType);
//
//	// If the import failed, report it
//	if(!scene) {
//		int AssimpLoadFileError = 0;
//		assert(AssimpLoadFileError);
//	}
//	
//	
//}