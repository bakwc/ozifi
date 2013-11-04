#pragma once

#include <vector>
#include <string>
#include <assert.h>
#include <utils/types.h>
#include <unordered_map>

namespace NNaiveBayesClassifier {

template <typename T>
class TClassifier {
public:
    TClassifier(size_t classesNumber) {
        ClassesProb.resize(classesNumber, 0);
        Prob.resize(classesNumber);
    }
    void AddTrainData(const std::vector<T>& data, size_t classNumber) {
        assert(classNumber < ClassesProb.size());
        ++ClassesProb[classNumber];
        std::unordered_map<T, float>& currentClass = Prob[classNumber];
        for (size_t i = 0; i < data.size(); ++i) {
            ++currentClass[data[i]];
        }
    }
    void Normalize() {  // should be called after training
        size_t totalCount = 0;
        for (size_t i = 0; i < ClassesProb.size(); ++i) {
            size_t currentClassNumber = ClassesProb[i];
            totalCount += currentClassNumber;
            std::unordered_map<T, float>& currentClass = Prob[i];
            typename std::unordered_map<T, float>::iterator it;
            for (it = currentClass.begin(); it != currentClass.end(); ++it) {
                it->second /= currentClassNumber;
            }
        }
        for (size_t i = 0; i < ClassesProb.size(); ++i) {
            ClassesProb[i] /= totalCount;
        }
    }
    std::vector<float> Predict(const std::vector<T>& data) {
        std::vector<float> probs;
        for (size_t i = 0; i < ClassesProb.size(); ++i) {
            double prob = ClassesProb[i];
            std::unordered_map<T, float>& currentClass = Prob[i];
            typename std::unordered_map<T, float>::iterator it;
            for (size_t j = 0; j < data.size(); ++j) {
                if (currentClass.find(data[j]) == currentClass.end()) {
                    prob *= 0.001; // todo: move to log
                    continue;
                }
                prob *= currentClass[data[j]];
            }
            probs.push_back(prob);
        }
        return probs;
    }
    size_t PredictClass(const std::vector<T>& data) {
        std::vector<float> predictions = Predict(data);
        size_t max = 0;
        for (size_t i = 1; i < predictions.size(); ++i) {
            if (predictions[i] > predictions[max]) {
                max = i;
            }
        }
        return max;
    }
    std::string SerializeModel();
    void LoadModel(const std::string&);
private:
    std::vector<std::unordered_map<T, float> > Prob;
    std::vector<float> ClassesProb;
};

} // NNaiveBayesClassifier
