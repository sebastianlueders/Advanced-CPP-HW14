#include "IndentStream.h"

using std::cout;
using std::endl;

namespace mpcs {
ostream &indent(ostream &ostr)
{
    IndentStreamBuf *out = dynamic_cast<IndentStreamBuf *>(ostr.rdbuf());
	if (nullptr != out) {
        out->myIndent += 4;
    }
    return ostr;
}

ostream &unindent(ostream &ostr)
{
    IndentStreamBuf *out = dynamic_cast<IndentStreamBuf *>(ostr.rdbuf());
    out->myIndent -= 4;
    return ostr;
}
}



