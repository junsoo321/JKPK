#pragma once

#include <string>
#include <vector>

struct QuizQuestion
{
    std::string question;

    std::vector<std::string> answers;
};

extern std::vector<QuizQuestion> gQuizQuestions;
