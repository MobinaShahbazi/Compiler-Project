#include "Lexer.h"

namespace charinfo {

	LLVM_READNONE inline bool isWhitespace(char c) {
		return c == ' ' || c == '\t' || c == '\f' ||
			c == '\v' || c == '\r' || c == '\n';
	}

	LLVM_READNONE inline bool isDigit(char c) {
		return c >= '0' && c <= '9';
	}

	LLVM_READNONE inline bool isLetter(char c) {
		return (c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z');
	}

	LLVM_READNONE inline bool isOperator(char c) {
		return c == '+' || c == '-' || c == '*' ||
			c == '/' || c == '^' || c == '=' ||
			c == '<' || c == '>' || c == '!' ||
			c == ':' || c == ',';
	}
}


void Lexer::setBufferPtr(const char* buffer){
	BufferPtr = buffer;
}

const char* Lexer::getBufferPtr(){
	return BufferPtr;
}

void Lexer::formToken(Token& Tok, const char* TokEnd, Token::TokenKind Kind) {
	Tok.Kind = Kind;
	Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
	BufferPtr = TokEnd;
}
