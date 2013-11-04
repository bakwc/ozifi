// tests.cpp

#include <string>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>

#include <library/naive_bayes_classifier/classifier.h>

using namespace std;

vector<string> s2v(const std::string& str) {
    vector<string> result;
    boost::algorithm::split(result, str, boost::algorithm::is_any_of(" "));
    return result;
}

TEST(naive_bayes_classifier, Simple) {
    NNaiveBayesClassifier::TClassifier<string> classifier(2);

    classifier.AddTrainData(s2v("visit this site"), 0);
    classifier.AddTrainData(s2v("open link on this site"), 0);
    classifier.AddTrainData(s2v("click this link"), 0);
    classifier.AddTrainData(s2v("open link on this site"), 0);
    classifier.AddTrainData(s2v("you can visit that link"), 0);

    classifier.AddTrainData(s2v("hello how are you"), 1);
    classifier.AddTrainData(s2v("glad to hear you"), 1);
    classifier.AddTrainData(s2v("what is you name"), 1);
    classifier.AddTrainData(s2v("where are you from"), 1);

    classifier.Normalize();

    size_t c1 = classifier.PredictClass(s2v("do you like to play hockey"));
    size_t c2 = classifier.PredictClass(s2v("please open the link in my letter"));

    ASSERT_EQ(c1, 1);
    ASSERT_EQ(c2, 0);
}

int main(int argc, char **argv) {
    setlocale(LC_CTYPE, "en_US.UTF-8");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
