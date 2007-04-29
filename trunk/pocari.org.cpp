/***********************************************************************
 * Copyright (C) 2007 SUNAOKA Norifumi All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

#include "pocari.org.h"

/************************************************************************
 * tokenizer
 ***********************************************************************/
void pocari_org::tokenizer::parse(std::set<std::string>& tokens, const char *text, const char *sepalator)
{
	char *tok;
	
	tok = strtok((char *)text, sepalator);
	while (tok != NULL) {
		tokens.insert(tok);
		tok = strtok(NULL, sepalator);
	}
}

void pocari_org::tokenizer::parse(std::vector<std::string>& tokens, const char *text, const char *sepalator)
{
	char *tok;
	
	tok = strtok((char *)text, sepalator);
	while (tok != NULL) {
		tokens.push_back(tok);
		tok = strtok(NULL, sepalator);
	}
}

/************************************************************************
 * array
 ***********************************************************************/
std::string pocari_org::array::join(std::set<std::string>& array, std::string sepalator)
{
	std::string string;

	for (std::set<std::string>::iterator itr = array.begin(); itr != array.end(); ++itr) {
		string += *itr + sepalator;
	}
	
	return string.substr(0, string.size() - sepalator.size());
}

std::string pocari_org::array::join(std::vector<std::string>& array, std::string sepalator)
{
	std::string string;

	for (std::vector<std::string>::iterator itr = array.begin(); itr != array.end(); ++itr) {
		string += *itr + sepalator;
	}

	return string.substr(0, string.size() - sepalator.size());
}
