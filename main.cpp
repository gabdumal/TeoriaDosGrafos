#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <utility>
#include <tuple>
#include <iomanip>
#include <stdlib.h>
#include <chrono>
#include "Graph.h"
#include "Node.h"

using namespace std;

// Constantes
static const int OPTION_INVALID = -1;
static const int OPTION_EXIT = 0;
static const int OPTION_EXPORT = 1;

// Variáveis globais
bool directed = false, weightedEdge = false, weightNode = false;

/*  Names a graph according it's kind, either directed or not
 *   writes .dot file after the .txt input
 */
string exportGraphToDotFormat(Graph *graph)
{
    if (graph == nullptr)
        return "";

    Node *nextNode = graph->getFirstNode();
    string dot = "", connector;
    bool weightedEdge = graph->getWeightedEdge();

    dot += "strict ";
    if (graph->getDirected())
    {
        connector = " -> ";
        dot += "di";
    }
    else
        connector = " -- ";

    dot += "graph grafo {\n";
    while (nextNode != nullptr)
    {
        dot += "  " + to_string(nextNode->getLabel()) +
               " [weight = " + to_string(nextNode->getWeight()) + "];\n";
        nextNode = nextNode->getNextNode();
    }
    nextNode = graph->getFirstNode();
    while (nextNode != nullptr)
    {
        Edge *nextEdge = nextNode->getFirstEdge();
        while (nextEdge != nullptr)
        {
            dot += "\n  " + to_string(nextNode->getLabel()) + connector +
                   to_string(graph->getNodeById(nextEdge->getTargetId())->getLabel());
            if (weightedEdge)
                dot += " [weight = " + to_string(nextEdge->getWeight()) + "];";
            nextEdge = nextEdge->getNextEdge();
        }
        nextNode = nextNode->getNextNode();
    }
    dot += "\n}\n";

    return dot;
}

/*  Reads from .txt file
    it also evaluetas three possible natures of the graph, if it is: directed, weighted on edges, weighted on nodes
*/
Graph *leitura(ifstream &input_file, int directed, int weightedEdge, int weightedNode)
{
    // Preenchimento das variáveis globais
    ::directed = directed;
    ::weightedEdge = weightedEdge;
    ::weightNode = weightNode;

    // Variáveis para auxiliar na criação dos nós no Grafo
    int labelNodeSource;
    int labelNodeTarget;
    int order;
    bool obeyOrder = true;
    int invalidLines = 0;

    // Pegando a ordem do grafo
    input_file >> order;

    // Criando objeto grafo
    Graph *graph = new Graph(order, directed, weightedEdge, weightedNode);

    // Grafo SEM peso nos nós, e SEM peso nas arestas
    if (!graph->getWeightedEdge() && !graph->getWeightedNode())
    {
        while (obeyOrder && (input_file >> labelNodeSource >> labelNodeTarget))
        {
            graph->insertEdge(labelNodeSource, labelNodeTarget, 0);
            if (graph->getNodeIdCounter() >= order)
                obeyOrder = false;
        }
    }
    // Grafo SEM peso nos nós, mas COM peso nas arestas
    else if (graph->getWeightedEdge() && !graph->getWeightedNode())
    {
        float edgeWeight;

        while (obeyOrder && (input_file >> labelNodeSource >> labelNodeTarget >> edgeWeight))
        {
            graph->insertEdge(labelNodeSource, labelNodeTarget, edgeWeight);
            if (graph->getNodeIdCounter() >= order)
                obeyOrder = false;
        }
    }
    // Grafo COM peso nos nós, mas SEM peso nas arestas
    else if (graph->getWeightedNode() && !graph->getWeightedEdge())
    {
        float nodeSourceWeight, nodeTargetWeight;

        while (obeyOrder && (input_file >> labelNodeSource >> nodeSourceWeight >> labelNodeTarget >> nodeTargetWeight))
        {
            graph->insertEdge(labelNodeSource, labelNodeTarget, 0);
            graph->getNodeByLabel(labelNodeSource)->setWeight(nodeSourceWeight);
            graph->getNodeByLabel(labelNodeTarget)->setWeight(nodeTargetWeight);
            if (graph->getNodeIdCounter() >= order)
                obeyOrder = false;
        }
    }
    // Grafo COM peso nos nós, e COM peso nas arestas
    else if (graph->getWeightedNode() && graph->getWeightedEdge())
    {
        float nodeSourceWeight, nodeTargetWeight, edgeWeight;

        while (obeyOrder && (input_file >> labelNodeSource >> nodeSourceWeight >> labelNodeTarget >> nodeTargetWeight >> edgeWeight))
        {
            graph->insertEdge(labelNodeSource, labelNodeTarget, edgeWeight);
            graph->getNodeByLabel(labelNodeSource)->setWeight(nodeSourceWeight);
            graph->getNodeByLabel(labelNodeTarget)->setWeight(nodeTargetWeight);
            if (graph->getNodeIdCounter() >= order)
                obeyOrder = false;
        }
    }

    // insere nó invalido para cada entrada acima da ordem
    while (order > graph->getNodeIdCounter())
    {
        --invalidLines;
        graph->insertNode(invalidLines);
    }
    // apenas uma inserção mesmo com várias inserções acima da ordem
    /*while (order > graph->getNodeIdCounter())
    {
        graph->insertNode(-1);
    }*/

    return graph;
}

Graph *createAuxiliaryGraphFromFile(ifstream &input_file, string input_file_name, int *selectedOption, string *errors)
{
    if (input_file.is_open())
        return leitura(input_file, ::directed, ::weightedEdge, ::weightNode);
    else
    {
        *errors += "ERRO: Não foi possível abrir o arquivo de entrada " + input_file_name + "!\n";
        *selectedOption = OPTION_INVALID;
        return nullptr;
    }
}

Graph *readAuxiliaryGraph(int *selectedOption, string *errors)
{
    cout << "Digite o caminho do arquivo de entrada do grafo auxiliar:" << endl;
    string auxiliary_input_file_name;
    cin >> auxiliary_input_file_name;

    ifstream auxiliary_input_file;
    auxiliary_input_file.open(auxiliary_input_file_name, ios::in);

    Graph *auxiliaryGraph = createAuxiliaryGraphFromFile(auxiliary_input_file, auxiliary_input_file_name, selectedOption, errors);

    auxiliary_input_file.close();
    return auxiliaryGraph;
}

string createUnionGraph(Graph *firstGraph, Graph *secondGraph)
{
    Graph *thirdGraph;
    string dot = "";
    if (!firstGraph->getDirected())
    {
        //COPIA GRAFO1 PARA GRAFO 3
        thirdGraph = new Graph(0, firstGraph->getDirected(), firstGraph->getWeightedEdge(), firstGraph->getWeightedEdge());
        //aux 
        Node *finalNode = firstGraph->getFirstNode();
        Edge *nextEdge;
        Node *nodeByIdFromEdge;

        while (finalNode != NULL)
        {
            nextEdge = finalNode->getFirstEdge();
            while (nextEdge != NULL)
            {
                nodeByIdFromEdge = firstGraph->getNodeById(nextEdge->getTargetId());
                thirdGraph->insertEdge(finalNode->getLabel(), nodeByIdFromEdge->getLabel() , 0);
                nextEdge = nextEdge->getNextEdge();
            }
            finalNode = finalNode ->getNextNode();
        }
        //VERIFICA QUAIS RELAÇÕES ESTÃO NO SEGUNDO GRAFO E NAO ESTÃO NO TERCEIRO
        
        finalNode = secondGraph->getFirstNode();
        while (finalNode != NULL)
        {
            nextEdge = finalNode->getFirstEdge();
            while (nextEdge != NULL)
            {
                nodeByIdFromEdge = firstGraph->getNodeById(nextEdge->getTargetId());
                if(!thirdGraph->existEdge(finalNode->getLabel(), nodeByIdFromEdge->getLabel()))
                {
                    thirdGraph->insertEdge(finalNode->getLabel(), nodeByIdFromEdge->getLabel() , 0);
                }
                nextEdge = nextEdge->getNextEdge();
            }
            finalNode = finalNode ->getNextNode();
        }
    }
    dot = exportGraphToDotFormat(thirdGraph);
    delete thirdGraph;
    thirdGraph = NULL;
    return dot;
}

/*  Prints the graph on terminal window
 *   at the end returns a boolean which is used either to export the graph or not
 */
bool showGraph(string dot)
{
    string response = "";

    cout << "GRAFO" << endl
         << "-----" << endl
         << dot << endl;
    while (response != "S" && response != "s" && response != "N" && response != "n")
    {
        cout << "Deseja exportar este grafo? Isso vai apagar o conteúdo atual do arquivo [S/n] ";
        cin >> response;
    }
    cout << endl
         << endl;
    return (response == "S" || response == "s");
}

/*  Shows a menu on terminal windows with 10 options
 *   only two of them works: number one wich prints the graph -on terminal- and number zero wich ends the program
 *   note that the other ones are not suitable for this project
 *   it also shows an error message in case of an incorrect input such letters or numbers others than 0-10
 */
int menu(string *errors)
{
    string selectedOption;

    cout << "MENU" << endl;
    cout << "----" << endl;
    cout << "[1] Imprimir Grafo de Entrada" << endl;
    cout << "[2] Gerar Grafo Interseção" << endl;
    cout << "[3] Gerar Grafo União" << endl;
    cout << "[4] Gerar Grafo Diferença" << endl;
    cout << "[5] Gerar Rede PERT" << endl;
    cout << "[0] Sair" << endl;

    cin >> selectedOption;

    try
    {
        return stoi(selectedOption);
    }
    catch (const std::invalid_argument &)
    {
        *errors = "ERRO: Letras são inválidas. Digite um número inteiro entre 0 e 10!\n\n";
        return OPTION_INVALID;
    }
    catch (const std::out_of_range &)
    {
        *errors == "ERRO: Fora dos limites. Digite um número inteiro entre 0 e 10!\n\n";
        return OPTION_INVALID;
    }
}

/*  Calls a function according with the entrance option given by user
 *   like the menu, only one and zero matters
 */
string selectOption(int *selectedOption, string *errors, Graph *firstGraph)
{
    string dot = "";
    int option = *selectedOption;
    switch (option)
    {
    case 0:
    {
        *selectedOption = OPTION_EXIT;
        break;
    }
    // Imprimir grafo de entrada
    case 1:
    {
        dot = exportGraphToDotFormat(firstGraph);
        break;
    }
    // Grafo interseção
    case 2:
    {

        break;
    }
    // Grafo união
    case 3:
    {
        Graph *secondGraph = readAuxiliaryGraph(selectedOption, errors);
        dot = createUnionGraph(firstGraph, secondGraph);

        delete secondGraph;
        secondGraph = nullptr;
        break;
    }
    // Grafo diferença
    case 4:
    {

        break;
    }
    // Rede Pert
    case 5:
    {

        break;
    }
    default:
    {
        *selectedOption = OPTION_INVALID;
    }
    }
    return dot;
}

/*  cleans terminal windows
 *   evaluates if the graph will be exported or not based on value of "shouldExport"
 *   also prints on terminal an error message in case of failure opening output file
 */
int mainMenu(string outputFileName, Graph *graph)
{
    string dot = "", errors = "";
    int selectedOption = OPTION_INVALID;

    // Loop de interface
    while (selectedOption != OPTION_EXIT)
    {

        // Impressão do grafo resultante, caso opção não seja inválida
        if (selectedOption != OPTION_INVALID)
        {
            bool shouldExport = showGraph(dot);
            if (shouldExport)
            {
                ofstream output_file;
                output_file.open(outputFileName, ios::out | ios::trunc);
                output_file << dot;
                output_file.close();
            }
        }

        system("clear");

        // Imprime erros
        cout << errors;
        errors = "";

        // Imprime menu de opções
        selectedOption = menu(&errors);
        dot = selectOption(&selectedOption, &errors, graph);
    }
    return 0;
}

/*  Verifies each parameter of the command line,
 *   if it is set properly calls menu function
 *   else it shows the expected model just beforing ending the program
 */
int main(int argc, char const *argv[])
{
    // Verifica se todos os argumentos foram fornecidos
    if (argc != 6)
    {
        cout << "ERRO: Espera-se: ./<program_name> <input_file> <output_file> <directed> <weighted_edge> <weighted_node>" << endl;
        return 1;
    }

    string program_name(argv[0]);
    string input_file_name(argv[1]);
    string output_file_name(argv[2]);

    // Abrindo arquivos de entrada e saída
    ifstream input_file;
    ofstream output_file;
    input_file.open(input_file_name, ios::in);

    Graph *graph;

    if (input_file.is_open())
        graph = leitura(input_file, atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    else
    {
        cout << "ERRO: Não foi possível abrir o arquivo de entrada " << input_file_name << "!" << endl;
        return 1;
    }

    int endingCode = mainMenu(output_file_name, graph);

    // Fechando arquivo de entrada
    input_file.close();

    // Fechando arquivo de saída
    output_file.close();

    return endingCode;
}

/*
 *  Comando para gerar imagem do grafo
    dot -Tpng output.dot -o graph.png
*/