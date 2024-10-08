#ifndef LEXER_H
#define LEXER_H

#include "llvm/ADT/StringRef.h"        // encapsulates a pointer to a C string and its length
#include "llvm/Support/MemoryBuffer.h" // provides read-only access to a block of memory, filled

class Lexer;

class Token {
    friend class Lexer;
    public:
		enum TokenKind : unsigned short {
			semi_colon,            // ;
			unknown,        // unknown token
			ident,          // identifier like a, b, c, d, etc.
            number,         // number like 1, 2, 3, 4, etc.
			comma,          // ,
			unery_plus,     // ++
            unery_minus,    // --
            plus,           // +
			minus,          // -
			star,           // *
			slash,          // /
			power,          // ^
			l_paren,        // (
			r_paren,        // )
			plus_equal,     // +=
			minus_equal,    // -=
			star_equal,     // *=
			slash_equal,    // /=
			mod_equal,		// %=
			equal,          // =
			equal_equal,    // ==
			not_equal,      // !=
			less,           // <
			less_equal,     // <=
			greater,        // >
			greater_equal,  // >=
			space,          // space
			new_line,       // \n
			KW_int,         // int
			KW_bool,        // bool
            KW_print,       // print
            KW_if,          // if
			KW_elseif,      // else if
			KW_else,        // else
			KW_for,         // for
            KW_while,       // while
			KW_and,         // and
			KW_or,          // or
			KW_true,        // true
			KW_false,       // false
			eof,            // end of file
			mod,            // %
			start_comment,  // /* 
            end_comment,    // */
            KW_begin,		// {
			KW_end			// }
		};


private:
	TokenKind Kind;         // <type of token,>
	llvm::StringRef Text;   // <,token context>
public:
	TokenKind getKind() const { return Kind; }
	llvm::StringRef getText() const { return Text; }

	bool is(TokenKind K) const { return Kind == K; }

	bool isOneOf(TokenKind K1, TokenKind K2) const {   // kind="+" isOneOf(plus, minus) -> true
		return is(K1) || is(K2);
	}

	template <typename... Ts>                           // variadic template
	bool isOneOf(TokenKind K1, TokenKind K2, Ts... Ks)  // can take several inputs
		const {
		return is(K1) || isOneOf(K2, Ks...);
	}
};

class Lexer {
	const char* BufferStart;
	const char* BufferPtr;

public:
	Lexer(const llvm::StringRef& Buffer) {    // constructor scans the whole context
		BufferStart = Buffer.begin();
		BufferPtr = BufferStart;
	}

	void next(Token& token);                       // gets next token
	void setBufferPtr(const char* buffer);
	const char* getBufferPtr(){return BufferPtr;};

private:
	void formToken(Token& Result, const char* TokEnd,
		Token::TokenKind Kind);
};
#endif