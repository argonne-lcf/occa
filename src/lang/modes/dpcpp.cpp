#include <occa/tools/string.hpp>
#include <occa/lang/modes/dpcpp.hpp>
#include <occa/lang/modes/okl.hpp>
#include <occa/lang/modes/oklForStatement.hpp>
#include <occa/lang/builtins/attributes.hpp>
#include <occa/lang/builtins/types.hpp>

namespace occa {
  namespace lang {
    namespace okl {
      dpcppParser::dpcppParser(const occa::properties &settings_) :
        withLauncherLambda(settings_),
        constant("__constant__", qualifierType::custom),
        global("__global__", qualifierType::custom),
        device("__device__", qualifierType::custom),
        shared("__shared__", qualifierType::custom) {

        okl::addAttributes(*this);
      }

      void dpcppParser::onClear() {
        launcherClear();
      }

      void dpcppParser::beforePreprocessing() {
        preprocessor.addCompilerDefine("OCCA_USING_GPU", "1");
      }

      void dpcppParser::beforeKernelSplit() {
        updateConstToConstant();

        if (!success) return;
        setFunctionQualifiers();

        if (!success) return;
        setSharedQualifiers();
      }

      void dpcppParser::afterKernelSplit() {
        addBarriers();

        if (!success) return;
        setupKernels();
      }

      std::string dpcppParser::getOuterIterator(const int loopIndex) {
        std::string name = "get_global_id(";
        name +=  loopIndex +"*";
        return name;
      }

      std::string dpcppParser::getInnerIterator(const int loopIndex) {
        std::string name = "threadIdx.";
        name += 'x' + (char) loopIndex;
        return name;
      }

      void dpcppParser::updateConstToConstant() {
        const int childCount = (int) root.children.size();
        for (int i = 0; i < childCount; ++i) {
          statement_t &child = *(root.children[i]);
          if (child.type() != statementType::declaration) {
            continue;
          }
          declarationStatement &declSmnt = ((declarationStatement&) child);
          const int declCount = declSmnt.declarations.size();
          for (int di = 0; di < declCount; ++di) {
            variable_t &var = *(declSmnt.declarations[di].variable);
            if (var.has(const_) && !var.has(typedef_)) {
              var -= const_;
              var += constant;
            }
          }
        }
      }

      void dpcppParser::setFunctionQualifiers() {
/*        statementPtrVector funcSmnts;
        findStatementsByType(statementType::functionDecl,
                             root,
                             funcSmnts);

        const int funcCount = (int) funcSmnts.size();
        for (int i = 0; i < funcCount; ++i) {
          functionDeclStatement &funcSmnt = (
            *((functionDeclStatement*) funcSmnts[i])
          );
          if (funcSmnt.hasAttribute("kernel")) {
            continue;
          }
          vartype_t &vartype = funcSmnt.function.returnType;
          vartype.qualifiers.addFirst(vartype.origin(),
                                      device);
        }*/
      }

      void dpcppParser::setSharedQualifiers() {
        statementExprMap exprMap;
        findStatements(statementType::declaration,
                       exprNodeType::variable,
                       root,
                       sharedVariableMatcher,
                       exprMap);

        statementExprMap::iterator it = exprMap.begin();
        while (it != exprMap.end()) {
          declarationStatement &declSmnt = *((declarationStatement*) it->first);
          const int declCount = declSmnt.declarations.size();
          for (int i = 0; i < declCount; ++i) {
            variable_t &var = *(declSmnt.declarations[i].variable);
            if (!var.hasAttribute("shared")) {
              continue;
            }
            var += shared;
          }
          ++it;
        }
      }

      void dpcppParser::addBarriers() {
        statementPtrVector statements;
        findStatementsByAttr(statementType::empty,
                             "barrier",
                             root,
                             statements);

        const int count = (int) statements.size();
        for (int i = 0; i < count; ++i) {
          // TODO 1.1: Implement proper barriers
          emptyStatement &smnt = *((emptyStatement*) statements[i]);

          statement_t &barrierSmnt = (
            *(new expressionStatement(
                smnt.up,
                *(new identifierNode(smnt.source,
                                     " __syncthreads()"))
              ))
          );

          smnt.up->addBefore(smnt,
                             barrierSmnt);

          smnt.up->remove(smnt);
          delete &smnt;
        }
      }

      void dpcppParser::setupKernels() {
        statementPtrVector kernelSmnts;
        findStatementsByAttr(statementType::functionDecl,
                             "kernel",
                             root,
                             kernelSmnts);


        const int kernelCount = (int) kernelSmnts.size();
        for (int i = 0; i < kernelCount; ++i) {
          functionDeclStatement &kernelSmnt = (
            *((functionDeclStatement*) kernelSmnts[i])
          );
          setKernelQualifiers(kernelSmnt);
          if (!success) return;
        }
	strVector headers;
        const bool includingStd = settings.get("serial/include_std", true);
        headers.push_back("include <CL/sycl.hpp>\n");

        const int headerCount = (int) headers.size();
        for (int i = 0; i < headerCount; ++i) {
          std::string header = headers[i];
          // TODO 1.1: Remove hack after methods are properly added
          directiveToken token(root.source->origin,
                               header);
          root.addFirst(
            *(new directiveStatement(&root, token))
          );
        }

      }

      void dpcppParser::setKernelQualifiers(functionDeclStatement &kernelSmnt) {
        vartype_t &vartype = kernelSmnt.function.returnType;
        vartype.qualifiers.addFirst(vartype.origin(),
                                    externC);


        function_t &func = kernelSmnt.function;

        const int argCount = (int) func.args.size();

        variable_t queueArg(syclQueuePtr, "q");

        func.addArgument(queueArg);//const identifierToken &typeToken_, const type_t &type_
        variable_t ndRangeArg(syclNdRangePtr, "ndrange");

        func.addArgument(ndRangeArg);//const identifierToken &typeToken_, const type_t &type_

        for (int i = 0; i < argCount; ++i) {
                func.addArgument(*func.removeArgument(0));
        }

      }

      bool dpcppParser::sharedVariableMatcher(exprNode &expr) {
        return expr.hasAttribute("shared");
      }
    }
  }
}
