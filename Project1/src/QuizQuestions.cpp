#pragma execution_character_set("utf-8")
#include "QuizQuestions.hpp"

// correctIndex: 0=A, 1=B, 2=C, 3=D
std::vector<QuizQuestion> gQuizQuestions =
{
    {
        "다음 중 선형 구조가 아닌 것은?",
        {
            "연결 리스트 (linked list)",
            "스택 (stack)",
            "큐 (queue)",
            "트리 (tree)"
        },
        3
    },
    {
        "다음 중 스택(Stack)을 활용하는 예시가 아닌 것은?",
        {
            "함수 호출 스택 (Call Stack)",
            "브라우저 뒤로가기",
            "수식의 괄호 검사",
            "프린터 출력 대기열"
        },
        3
    },
    {
        "큐(Queue)의 데이터 처리 순서로 올바른 것은?",
        {
            "후입선출 (LIFO)",
            "우선순위 순서",
            "선입선출 (FIFO)",
            "무작위 순서"
        },
        2
    },
};
