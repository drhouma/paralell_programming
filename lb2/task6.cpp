#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

std::string vocals = "eyuioa";

bool IsVocal(char character) {
    return vocals.find(character) != std::string::npos;
}

struct mapData {
    int dataSize;
    void *data;
    std::map<char, int> *res;

    void Init(int dataSize_, void *data_) {
        dataSize = dataSize_;
        data = data_;
        res = new std::map<char, int>;
    }
};

struct reduceData {
    std::vector<int> *dataVector;
    int *result;

    void Init(std::vector<int> *dataVector_) {
        dataVector = dataVector_;
        result = new int;
    }
};

// return dictionary with vocals as keys
// and amount of vocals as a value
void *MapFunction(void *data) {
    mapData *mdata = (mapData *)data;
    char *data_ptr = (char *)(mdata->data);
    int dataSize = mdata->dataSize;
    int i = 0;
    std::map<char, int> res;
    for (char *iter = data_ptr; i != dataSize; iter++, i++) {
        if (IsVocal(*iter)) {
            res[*iter]++;
        }
    }
    *(mdata->res) = res;
    delete data;
}

void *ReduceFunction(void *data) {
    reduceData *rdata = (reduceData *)data;

    int sum{0};
    for (auto elem : *(rdata->dataVector)) {
        sum += elem;
    }
    *(rdata->result) = sum;
    delete data;
}

// dataSize in bytes
void MapReduce(void *data, int dataSize, void *(*mapFunc)(void *), void *(*reduceFunc)(void *), int maxThreads) {
    // mapping data part, map data stores in maps
    int chunkSize = dataSize / maxThreads;
    std::cout << chunkSize << std::endl;
    void *currentDataChunk = data;
    std::vector<std::map<char, int> *> maps;  // allocate mem
    std::vector<pthread_t> threads;

    pthread_attr_t attr;

    for (int i = 0; i < maxThreads; i++) {
        pthread_t t;
        currentDataChunk = (char *)data + i * chunkSize;
        if (i == maxThreads - 1) {
            chunkSize = chunkSize + dataSize % maxThreads;
        }

        mapData *data_arg = new mapData;
        data_arg->Init(chunkSize, currentDataChunk);
        maps.push_back(data_arg->res);
        for (int i = 0; i < chunkSize; i++) {
            std::cout << *((char *)data_arg->data + i);
        }
        std::cout << '\n';

        pthread_create(&t, NULL, mapFunc, data_arg);
        threads.push_back(t);
    }
    for (auto t : threads) {
        pthread_join(t, NULL);
    }

    // vocals : list of values
    std::map<char, std::vector<int> > mappedResults;

    // sort
    for (auto elem : maps) {
        for (auto it = elem->begin(); it != elem->end(); ++it) {
            mappedResults[it->first].push_back(it->second);
        }
    }

    // for (auto it = mappedResults.begin(); it != mappedResults.end(); ++it) {
    //     std::cout << "vocal: " << it->first << std::endl;
    //     for (auto elem : it->second) {
    //         std:: cout << " " << elem;
    //     }
    //     std::cout << std::endl;
    // }

    threads.clear();

    std::vector<int *> results;  // allocate mem
    for (auto it = mappedResults.begin(); it != mappedResults.end(); ++it) {
        pthread_t t;

        reduceData *data_arg = new reduceData;
        data_arg->Init(&(it->second));
        results.push_back(data_arg->result);

        pthread_create(&t, NULL, reduceFunc, data_arg);
        threads.push_back(t);
    }

    for (auto t : threads) {
        pthread_join(t, NULL);
    }

    int res{0};
    for (auto elem : results) {
        res += *elem;
        delete elem;
    }

    for (auto elem : maps) {
        delete elem;
    }
    std::cout << "total number of vocals in data: " << res << std::endl;
}

int main() {
    std::string data;
    // data =
    //     "kerjglkwrejg;lrwegl"
    //     ";werg;lwnergnwrkgnhriuwyt[pqjoyuqetruiwqeoroqpew"
    //     "[irpuoyiwqeroupqewruoyqweiurtqiweorpqkwejrho]]f";
    // 5e, 10a
    data = "aattteetttaattttteetttttaahhhhehhhhaahhhhhhaa";
    MapReduce((void *)data.c_str(), data.size(), MapFunction, ReduceFunction, 6);
}