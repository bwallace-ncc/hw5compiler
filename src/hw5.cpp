/*======================================================================================================================================================================
 * Homework 5 - Compiler Part 2 (Syntax Checker)
 * John Wolf and Brandon Wallace
 * CSCE 306
 * Dr. St.Clair
 * 11/9/20
 *====================================================================================================================================================================*/
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
using namespace std;

//======================================================================================================================================================================
/*
 * I have this space here for notes and the like. Also, above main is a section for putting in methods to test the program. That way they're all in one spot
 * when we go to delete them before turning this in
 *
 * Notes:
 *  -I think it just falls down to testing with bigger test files and seeing if anything fails. It seems to be working pretty well and we just need to test for holes
 *
 * Issues:
 *
 * To do:
 * -Write a more robust test method/file?
 */
//======================================================================================================================================================================
class SyntaxAnalyzer{
    private:
        vector<string> lexemes;
        vector<string> tokens;
        vector<string>::iterator lexitr;
        vector<string>::iterator tokitr;

        // map of variables to datatype (i.e. sum t_integer)
        // sets this up for stage 3
        map<string, string> symboltable;

        // other private methods
        bool vdec();
        int vars();
        bool stmtlist();
        int stmt();
        bool ifstmt();
        bool elsepart();
        bool whilestmt();
        bool assignstmt();
        bool inputstmt();
        bool outputstmt();
        bool expr();
        bool simpleexpr();
        bool term();
        bool logicop();
        bool arithop();
        bool relop();
        std::istream& getline_safe(std::istream& input, std::string& output);
    public:
        SyntaxAnalyzer(istream& infile);
        // pre: 1st parameter consists of an open file containing a source code's
        //	valid scanner output.  This data must be in the form
        //			token : lexeme
        // post: the vectors have been populated

        bool parse();
        // pre: none
        // post: The lexemes/tokens have been parsed and the symboltable created.
        // If an error occurs, a message prints indicating the token/lexeme pair
        // that caused the error.  If no error occurs, the symboltable contains all
        // variables and datatypes.

        //for testing
        void runTest(stringstream&);
};
SyntaxAnalyzer::SyntaxAnalyzer(istream& infile){
    string line, tok, lex;
    int pos;
    getline_safe(infile, line);
    bool valid = true;
    while(!infile.eof() && (valid)){
        pos = line.find(":");
        tok = line.substr(0, pos);
        lex = line.substr(pos+1, line.length());
        cout << pos << " " << tok << " " << lex << endl;
        tokens.push_back(tok);
        lexemes.push_back(lex);
        getline_safe(infile, line);
    }
    tokitr = tokens.begin();
    lexitr = lexemes.begin();
}

bool SyntaxAnalyzer::parse(){
    if (vdec()){
        if (tokitr!=tokens.end() && *tokitr=="t_main"){
            tokitr++; lexitr++;
            if (tokitr!=tokens.end() && stmtlist()){
            	if (tokitr!=tokens.end()) // should be at end token
                	if (*tokitr == "t_end"){
                		tokitr++; lexitr++;
                		if (tokitr==tokens.end()){  // end was last thing in file
                			cout << "Valid source code file" << endl;
                			return true;
                		}
                		else{
                			cout << "end came too early" << endl;
                		}
                	}
                	else{
                		cout << "invalid statement ending code" << endl;
                }
                else{
                	cout << "no end" << endl;
                }
            }
            else{
            	cout << "bad/no stmtlist" << endl;
            }
        }
        else{
        	cout << "no main" << endl;
        }
    }
    else{
    	cout << "bad var list" << endl;
    }
    return false;

}

bool SyntaxAnalyzer::vdec(){

    if (tokitr!=tokens.end() && *tokitr != "t_var")
        return true;
    else{
        tokitr++; lexitr++;
        int result = 0;   // 0 - valid, 1 - done, 2 - error
        result = vars();
        if (result == 2)
            return false;
        while (result == 0){
            if (tokitr!=tokens.end())
                result = vars(); // parse vars
        }

        if (result == 1)
            return true;
        else
            return false;
    }
}

int SyntaxAnalyzer::vars(){
    int result = 0;  // 0 - valid, 1 - done, 2 - error
    string temp;
    if (tokitr!=tokens.end() && *tokitr == "t_integer"){
        temp = "t_integer";
        tokitr++; lexitr++;
    }
    else if (tokitr!=tokens.end() && *tokitr == "t_string"){
        temp = "t_string";
        tokitr++; lexitr++;
    }
    else
        return 1;
    bool semihit = false;
    while (tokitr != tokens.end() && result == 0 && !semihit){
        if (*tokitr == "t_id"){
            symboltable[*lexitr] = temp;
            tokitr++; lexitr++;
            if (tokitr != tokens.end() && *tokitr == "s_comma"){
                tokitr++; lexitr++;
            }
            else if (tokitr != tokens.end() && *tokitr == "s_semi"){
                semihit = true;
                tokitr++; lexitr++;
            }
            else
                result = 2;
        }
        else{
            result = 2;
        }
    }
    return result;
}

bool SyntaxAnalyzer::stmtlist(){
    int result = stmt();

    while (result == 1){
    	result = stmt();
    }
    if (result == 0)
        return false;
    else
        return true;
}
int SyntaxAnalyzer::stmt(){  // returns 1 or 2 if valid, 0 if invalid
	if (tokitr!=tokens.end() && *tokitr == "t_if"){
        tokitr++; lexitr++;
        if (ifstmt()) return 1;
        else return 0;
    }
    else if (tokitr!=tokens.end() && *tokitr == "t_while"){
        tokitr++; lexitr++;
        if (whilestmt()) return 1;
        else return 0;
    }
    else if (tokitr!=tokens.end() && *tokitr == "t_id"){  // assignment starts with identifier
        tokitr++; lexitr++;
        if (assignstmt()) return 1;
        else return 0;
    }
    else if (tokitr!=tokens.end() && *tokitr == "t_input"){
        tokitr++; lexitr++;
        if (inputstmt()) return 1;
        else return 0;
    }
    else if (tokitr!=tokens.end() && *tokitr == "t_output"){
        tokitr++; lexitr++;
        if (outputstmt()) return 1;
        else return 0;
    }
    return 2;  //stmtlist can be null
}

/*
 * John Wolf
 * Brandon Wallace
 *
 * pre: grammar calls for a if statement
 * post: gives boolean determining if pointer is pointing at a syntactically correct if statement
 *
 * @param: null
 * @return: bool isIfStmt
 */
bool SyntaxAnalyzer::ifstmt(){
	if (tokitr != tokens.end() && *tokitr == "s_lparen")
	{
		tokitr++; lexitr++;
		if (expr())
		{
			if (tokitr != tokens.end() && *tokitr == "s_rparen")
			{
				tokitr++; lexitr++;
				if (tokitr != tokens.end() && *tokitr == "t_then")
				{
					tokitr++; lexitr++;
					if (stmtlist())
					{
						if (elsepart())
						{
							if (tokitr != tokens.end() && *tokitr == "t_end")
							{
								tokitr++; lexitr++;
								if (tokitr != tokens.end() && *tokitr == "t_if")
								{
									tokitr++; lexitr++;
									return true;
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
	// we will write this together in class
}

bool SyntaxAnalyzer::elsepart(){
    if (tokitr!=tokens.end() && *tokitr == "t_else"){
        tokitr++; lexitr++;
        if (stmtlist())
            return true;
        else
            return false;
    }
    return true;   // elsepart can be null
}
/*
 * John Wolf
 *
 * pre: grammar calls for a while statement
 * post: gives boolean determining if pointer is pointing at a syntactically correct while statement
 *
 * @param: null
 * @return: bool isWhileStmt
 */
bool SyntaxAnalyzer::whilestmt()
{
	if(tokitr != tokens.end() && *tokitr == "s_lparen")
	{
		tokitr++; lexitr++;
		if(expr())
		{
			if(tokitr != tokens.end() && *tokitr == "s_rparen")
			{
				tokitr++; lexitr++;
				if(tokitr != tokens.end() && *tokitr == "t_loop")
				{
					tokitr++; lexitr++;
					if(stmtlist())
					{
						if(tokitr != tokens.end() && *tokitr == "t_end")
						{
							tokitr++; lexitr++;
							if(tokitr != tokens.end() && *tokitr == "t_loop")
							{
								tokitr++; lexitr++;
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

/*
 * Brandon Wallace
 *
 * pre: grammar calls for an assignment statement
 * post: gives boolean determining if pointer is pointing at a syntactically correct assignment statement
 *
 * @param: null
 * @return: bool isAssignStmt
 */
bool SyntaxAnalyzer::assignstmt(){
	if(tokitr != tokens.end() && *tokitr == "s_assign"){
		tokitr++; lexitr++;
		if(expr()){
			if(tokitr != tokens.end() && *tokitr == "s_semi"){
				tokitr++; lexitr++;
				return true;
			}
		}
	}
	return false;
}
bool SyntaxAnalyzer::inputstmt(){
    if (tokitr != tokens.end() && *tokitr == "s_lparen"){
        tokitr++; lexitr++;
        if (tokitr != tokens.end() && *tokitr == "t_id"){
            tokitr++; lexitr++;
            if (tokitr != tokens.end() && *tokitr == "s_rparen"){
                tokitr++; lexitr++;
                return true;
            }
        }
    }
    return false;
}

/*
 * Brandon Wallace
 *
 * pre: grammar calls for an output statement
 * post: gives boolean determining if pointer is pointing at a syntactically correct output statement
 *
 * @param: null
 * @return: bool isOutputStmt
 */
bool SyntaxAnalyzer::outputstmt(){
	if(expr()){
		return true;
	}
	else if(tokitr != tokens.end() && *tokitr == "t_string"){
		tokitr++; lexitr++;
		return true;
	}
	return false;
}

bool SyntaxAnalyzer::expr()
{
    if (simpleexpr())
    {
		if (logicop())
		{
			if (simpleexpr()) {return true;}
			else {return false;}
		}
		else {return true;}
    }
    else {return false;}
}
/*
 * John Wolf
 *
 * pre: grammar calls for a simple expression
 * post: gives boolean determining if pointer is pointing at a syntactically correct simple expression
 *
 * @param: null;
 * @return bool isSimpleExpression
 */
bool SyntaxAnalyzer::simpleexpr()
{
	if(term())
	{
		if(arithop())
		{
			if(term()) {return true;}
			return false;
		}
		else if(relop())
		{
			if(term()) {return true;}
			return false;
		}
		else {return true;}
	}
	return false;
}
bool SyntaxAnalyzer::term()
{
    if ((*tokitr == "t_int")
	|| (*tokitr == "t_str")
	|| (*tokitr == "t_id"))
    {
    	tokitr++; lexitr++;
    	return true;
    }
    else
    {
    	if (tokitr!=tokens.end() && *tokitr == "s_lparen")
    	{
    		tokitr++; lexitr++;
    		if (expr())
    		{
    			if (tokitr!=tokens.end() && *tokitr == "s_rparen")
    			{
					tokitr++; lexitr++;
					return true;
				}
    		}
    	}
    }
    return false;
}

bool SyntaxAnalyzer::logicop(){
    if ((*tokitr == "s_and") || (*tokitr == "s_or")){
        tokitr++; lexitr++;
        return true;
    }
    else
        return false;
}

bool SyntaxAnalyzer::arithop(){
    if ((*tokitr == "s_mult") || (*tokitr == "s_plus") || (*tokitr == "s_minus")
        || (*tokitr == "s_div")	|| (*tokitr == "s_mod")){
        tokitr++; lexitr++;
        return true;
    }
    else
        return false;
}

bool SyntaxAnalyzer::relop(){
    if ((*tokitr == "s_lt") || (*tokitr == "s_gt") || (*tokitr == "s_ge")
        || (*tokitr == "s_eq") || (*tokitr == "s_ne") || (*tokitr == "s_le")){
        tokitr++; lexitr++;
        return true;
    }
    else
    	return false;
}
std::istream& SyntaxAnalyzer::getline_safe(std::istream& input, std::string& output)
{
    char c;
    output.clear();

    input.get(c);
    while (input && c != '\n')
    {
        if (c != '\r' || input.peek() != '\n')
        {
            output += c;
        }
        input.get(c);
    }

    return input;
}

int main()
{
    ifstream infile("codelexemes.txt");
    if (!infile)
    {
    	cout << "error opening lexemes.txt file" << endl;
        exit(-1);
    }
    SyntaxAnalyzer sa(infile);
    sa.parse();
    return 1;
}
