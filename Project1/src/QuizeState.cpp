#include "QuizStage.hpp"

static std::string userInput;
static bool finished = false;
static bool correct = false;

static const char* question =
"for(int i=0;i<10;____)";

void StartQuizStage()
{
    userInput.clear();

    finished = false;
    correct = false;

    SDL_StartTextInput();
}

void HandleQuizEvent(SDL_Event* event)
{
    if (finished)
        return;

    if (event->type == SDL_TEXTINPUT)
    {
        userInput += event->text.text;
    }

    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.sym == SDLK_BACKSPACE)
        {
            if (!userInput.empty())
                userInput.pop_back();
        }

        if (event->key.keysym.sym == SDLK_RETURN)
        {
            if (
                userInput == "i++" ||
                userInput == "++i"
                )
            {
                correct = true;
            }

            finished = true;

            SDL_StopTextInput();
        }
    }
}

bool IsQuizFinished()
{
    return finished;
}

bool IsQuizCorrect()
{
    return correct;
}
