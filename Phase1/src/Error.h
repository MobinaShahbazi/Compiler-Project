#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include "Lexer.h"
using namespace std;

class Error {

public:
	static void SemiColonNotFound();
	static void DefineInsideScope();
	static void AssignmentEqualNotFound();
	static void AssignmentSidesNotEqual();
	static void VariableNameNotFound();
	static void BooleanValueExpected();
	static void NumberVariableExpected();
	static void EndNotSeenForIf();
	static void BeginExpectedAfterColon();
	static void EndCommentExpected();
	static void LeftParenthesisExpected();
	static void RightParenthesisExpected();
	static void OnlyIdentifierCanBePrinted();
	static void UnexpectedTokenAfterMinusOrPlus();

};

#endif
