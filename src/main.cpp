#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>

namespace {
bool FindNodeWithName(xmlNodePtr node, const char* name) {
  if (node->type == XML_ELEMENT_NODE) {
    if (!xmlStrcmp(node->name, (const xmlChar*)name)) {
      printf("Found <%s> tag\n", name);
      return true;
    }
  }
  for (xmlNodePtr cur = node->children; cur; cur = cur->next) {
    if (FindNodeWithName(cur, name))
      return true;
  }
  return false;
}
}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <xmlfile>\n", argv[0]);
    return 1;
  }

  // 初始化 libxml2 库
  xmlInitParser();
  LIBXML_TEST_VERSION

  // 加载 XML 文件
  xmlDocPtr doc = xmlReadFile(argv[1], NULL, 0);
  if (doc == NULL) {
    fprintf(stderr, "Document not parsed successfully.\n");
    return 1;
  }

  // 获取文档的根节点
  xmlNodePtr root = xmlDocGetRootElement(doc);

  // 检查是否存在 <hello> 标签
  bool found = FindNodeWithName(root, "hello");

  if (!found) {
    printf("<hello> tag not found\n");
  }

  // 清理
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return 0;
}