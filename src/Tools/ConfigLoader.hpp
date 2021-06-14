#pragma once

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

#include "Reflect.hpp"

class ConfigLoader {
public:
    static vector<Reflect::Element> LoadConfig(string fn)
    {
        freopen(fn.c_str(),"w",stdin);

        std::vector<Reflect::Element> elements;
        string line;
        while(true){
            cin >> line;
            if(line == "END")
                break;
            vector<Reflect::Data> add;
            while(true){
                cin >> line;
                if(line == "---")
                    break;
                Reflect::Data data;
                if(line == "f1"){
                    cin >> data.offset;
                    cin >> data.f[0];
                }
                else if(line == "f3"){
                    cin >> data.offset;
                    cin >> data.f[0] >> data.f[1] >> data.f[2];
                }
                else if(line == "i1"){
                    cin >> data.offset;
                    cin >> data.i[0];
                }
                else if(line == "s"){
                    cin >> data.offset;
                    cin >> data.s[0];
                }
                else {
                    int SaveConfigBadType = 0;
                    assert(SaveConfigBadType);
                }
                add.push_back(data);
            }
            elements.push_back(Reflect::Element{add});
        }
        
        fclose(stdin);

        return elements;
    }

    static void SaveConfig(string fn, vector<Reflect::Element> elements)
    {
        freopen(fn.c_str(),"w",stdout);

        for(const auto& ele: elements){
            const auto& datas = ele.datas;
            cout << "---\n";
            for(const auto& data: datas){
                if(data.type == Reflect::Type::FLOAT){
                    cout << "f1" << "\n";
                    cout << data.offset << "\n";
                    printf("%.2f\n", data.f[0]);
                }
                else if(data.type == Reflect::Type::FLOAT3){
                    cout << "f3" << "\n";
                    cout << data.offset << "\n";
                    printf("%.2f %.2f %.2f\n",data.f[0], data.f[1], data.f[2]);
                }
                else if(data.type == Reflect::Type::INT){
                    cout << "i1" << "\n";
                    cout << data.offset << "\n";
                    printf("%d\n", data.i[0]);
                }
                else if(data.type == Reflect::Type::STRING){
                    cout << "s" << "\n";
                    cout << data.offset << "\n";
                    cout << data.s << "\n";
                }   
                else {
                    // 
                    int SaveConfigBadType = 0;
                    assert(SaveConfigBadType);
                }
            }
            cout << "---\n";
        }
        cout << "END" << "\n";
        fclose(stdout);
    }
};