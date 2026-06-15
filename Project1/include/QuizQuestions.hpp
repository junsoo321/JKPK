#pragma once

#include <string>
#include <vector>

struct QuizQuestion
{
    std::string question;
    std::string options[4]; // A, B, C, D
    int correctIndex;       // 0=A, 1=B, 2=C, 3=D
};

extern std::vector<QuizQuestion> gQuizQuestions;
