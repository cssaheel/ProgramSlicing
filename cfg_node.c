#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg_node.h"

CfgNode *createCFG(AstNode *node_ast)
{
	CfgNode *entrynode = newCfgNode(Entry);
	CfgNode *exitnode = newCfgNode(Exit);
	NodeParameterList *first, *last;
	NodeParameterList *p;	/*用于节点释放空间时*/

	/*抽象语法树是从1开始编号的，这里把控制流图的入口和出口节点的id_of_ast的值都设为0,以示特殊*/
	entrynode->node_of_ast = exitnode->node_of_ast = NULL;

	/*为入口节点创建一个后继节点，路径值为true*/
	entrynode->successor = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	entrynode->successor->path_value = true;
	entrynode->successor->next = NULL;

	first = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	first->node_succ = entrynode->successor;
	first->next = NULL; 

	last = createNodeRecursively(first, node_ast);

	while (last)
	{
		last->node_succ->node_cfg = exitnode;

		p = last;
		last = last->next;
		free(p);
	}

	return entrynode;
}

NodeParameterList *createNodeRecursively(NodeParameterList *pre, AstNode *node_ast)
{
	NodeParameterList *path_return;
	AstNode *p;

	if (!node_ast)	/*node_ast为NULL*/
		return pre;

	if (node_ast->nodetype_ast == Statement)
	{
		/*赋值语句或表达式语句*/
		if (strcmp(node_ast->value.value_string, "assignment_statement") == 0
			|| strcmp(node_ast->value.value_string, "expression_statement") == 0
			|| strcmp(node_ast->value.value_string, "jump_statement") == 0)
			path_return = assign_express_jump_case(pre, node_ast);

		/*选择语句*/
		if (strcmp(node_ast->value.value_string, "selection_statement_no_else") == 0)
			path_return =  select_no_else_case(pre, node_ast);
		if (strcmp(node_ast->value.value_string, "selection_statement_with_else") == 0)
			path_return =  select_with_else_case(pre, node_ast);

		/*循环语句*/
		if (strcmp(node_ast->value.value_string, "iteration_statement") == 0)
			path_return =  iteration_case(pre, node_ast);

	}
	else
	{
		path_return = createNodeRecursively(pre, node_ast->firstchild);
		if (node_ast->firstchild)	/*有第一个孩子*/
		{
			p = node_ast->firstchild->nextsibling;
			while (p)
			{
				path_return = createNodeRecursively(path_return, p);
				p = p->nextsibling;	
			}
		}
	}

	return path_return;
}

NodeParameterList *assign_express_jump_case(NodeParameterList *pre, AstNode *node_ast)
{
	CfgNode *temp = newCfgNode(Assign_express_jump);
	NodeParameterList *path_return;
	NodeParameterList *p;	/*用于节点释放空间时*/

	temp->node_of_ast = node_ast;
	/*创建后继节点*/
	temp->successor = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	temp->successor->path_value = true;
	temp->successor->next = NULL;

	while (pre)
	{
		pre->node_succ->node_cfg = temp;

		p = pre;
		pre = pre->next;
		free(p);
	}

	path_return = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	path_return->node_succ = temp->successor;
	path_return->next = NULL;

	return path_return;
}

NodeParameterList *select_no_else_case(NodeParameterList *pre, AstNode *node_ast)
{
	CfgNode *ifnode = newCfgNode(Select);
	NodeParameterList *to_true_path, *true_path_return, *false_path;
	NodeParameterList *p;	/*用于节点释放空间时*/

	ifnode->node_of_ast = node_ast;
	/*选择语句有两个后继节点，一个路径值为true，另一个为false*/
	ifnode->successor = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	ifnode->successor->path_value = true;
	ifnode->successor->next = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	ifnode->successor->next->path_value = false;
	ifnode->successor->next->next = NULL;

	while (pre)
	{
		pre->node_succ->node_cfg = ifnode;
		
		p = pre;
		pre = pre->next;
		free(p);
	}

	to_true_path = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	to_true_path->node_succ = ifnode->successor;
	to_true_path->next = NULL;

	true_path_return = createNodeRecursively(to_true_path, 
			/*第五个孩子*/	node_ast->firstchild->nextsibling->nextsibling->nextsibling->nextsibling);

	/*将if节点的false路径插入true路径的返回链表中，即得到整个if语句的返回链表*/
	false_path = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	false_path->node_succ = ifnode->successor->next;
	/*头插法*/
	false_path->next = true_path_return;
	true_path_return = false_path;

	return true_path_return;
}

NodeParameterList *select_with_else_case(NodeParameterList *pre, AstNode *node_ast)
{
	CfgNode *ifnode = newCfgNode(Select);
	NodeParameterList *to_true_path, *to_false_path, *true_path_return, *false_path_return, *q;
	NodeParameterList *p;	/*用于节点释放空间时*/

	ifnode->node_of_ast = node_ast;

	/*if节点有两个后继节点，一个路径值为true，另一个为false*/
	ifnode->successor = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	ifnode->successor->path_value = true;
	ifnode->successor->next = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	ifnode->successor->next->path_value = false;
	ifnode->successor->next->next = NULL;

	while (pre)
	{
		pre->node_succ->node_cfg = ifnode;

		p = pre;
		pre = pre->next;
		free(p);
	}

	to_true_path = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	to_true_path->node_succ = ifnode->successor;
	to_true_path->next = NULL;

	to_false_path = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	to_false_path->node_succ = ifnode->successor->next;
	to_false_path->next = NULL;

	true_path_return = createNodeRecursively(to_true_path, 
			/*第五个孩子*/ node_ast->firstchild->nextsibling->nextsibling->nextsibling->nextsibling);
	false_path_return= createNodeRecursively(to_false_path, 
			/*第七个孩子*/ node_ast->firstchild->nextsibling->nextsibling->nextsibling->nextsibling->nextsibling->nextsibling);

	/*找到最后一个节点*/
	for (q = true_path_return; q->next; q = q->next);
	/*将false_path_return连接在其后，即得到整个if语句的返回链表*/
	q->next = false_path_return;

	return true_path_return;
}

NodeParameterList *iteration_case(NodeParameterList *pre, AstNode *node_ast)
{
	CfgNode *whilenode = newCfgNode(Iteration);
	NodeParameterList *to_true_path, *to_false_path, *true_path_return;
	NodeParameterList *p;	/*用于节点释放空间时*/

	whilenode->node_of_ast = node_ast;

	/*while节点有两个后继节点，一个路径值为true，另一个为false*/
	whilenode->successor = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	whilenode->successor->path_value = true;
	whilenode->successor->next = (CfgNodeList *)malloc(sizeof(CfgNodeList));
	whilenode->successor->next->path_value = false;
	whilenode->successor->next->next = NULL;

	while (pre)
	{
		pre->node_succ->node_cfg = whilenode;

		p = pre;
		pre = pre->next;
		free(p);
	}

	to_true_path = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	to_true_path->node_succ = whilenode->successor;
	to_true_path->next = NULL;

	to_false_path = (NodeParameterList *)malloc(sizeof(NodeParameterList));
	to_false_path->node_succ = whilenode->successor->next;
	to_false_path->next = NULL;

	true_path_return = createNodeRecursively(to_true_path, 
			/*第五个孩子*/ node_ast->firstchild->nextsibling->nextsibling->nextsibling->nextsibling);

	/*循环结尾指向自己*/
	while (true_path_return)
	{
		true_path_return->node_succ->node_cfg = whilenode;

		p = true_path_return;
		true_path_return = true_path_return->next;
		free(p);
	}

	return to_false_path;
}

CfgNode *newCfgNode(enum CfgNodeType type)
{
	CfgNode *node = (CfgNode *)malloc(sizeof(CfgNode));

	node->nodetype_cfg = type;

	node->visited = false;

	node->successor = NULL;

	return node;
}

void freeCfgNode(CfgNode *node)
{
	CfgNodeList *p, *q;

	/*因上面已经访问过一边，visited值都为true，所以这次反过来*/
	node->visited = false;

	p = node->successor;
	while (p)
	{
		if (p->node_cfg->visited)
			freeCfgNode(p->node_cfg);

		q = p;
		p = p->next;
		free(q);	/*释放后继节点列表所占空间*/
	}

	/*退出之前释放当前节点所占空间*/
	free(node);
}