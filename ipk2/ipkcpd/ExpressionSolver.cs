namespace ipkcpd;

public class ExpressionSolver : IExpressionSolver
{
    // Define a delegate type for your methods
    private delegate int ExecuteOperation(List<int> operands);

    // Define some methods that match the signature of the delegate
    private static int Add(List<int> operands)
    {
        var result = operands[0];
        operands.RemoveAt(0);
        foreach (var operand in operands)
        {
            result += operand;
        }
        return result;
    }

    private static int Subtract(List<int> operands)
    {
        var result = operands[0];
        operands.RemoveAt(0);
        foreach (var operand in operands)
        {
            result -= operand;
        }
        return result;
    }

    private static int Multiply(List<int> operands)
    {
        var result = operands[0];
        operands.RemoveAt(0);
        foreach (var operand in operands)
        {
            result *= operand;
        }
        return result;
    }

    private static int Divide(List<int> operands)
    {
        var result = operands[0];
        operands.RemoveAt(0);
        foreach (var operand in operands)
        {
            if (operand == 0)
                throw new InvalidExpressionException("Division by zero.");
            result /= operand;
        }
        return result;
    }

    private readonly Dictionary<char, ExecuteOperation> _operators = new();

    public ExpressionSolver()
    {
        _operators.Add('+', Add);
        _operators.Add('-', Subtract);
        _operators.Add('*', Multiply);
        _operators.Add('/', Divide);
    }



    public int Solve(string expression)
    {
        ValidateExpression(expression);
        return SolveExpression(expression);
    }

    private int SolveExpression(string expression)
    {
        var stack = new Stack<string>();

        for (var i = 0; i < expression.Length; i++)
        {
            if (expression[i] == '(' || expression[i] == ' ')
                continue;

            if (_operators.ContainsKey(expression[i])) // operator
                stack.Push(expression[i].ToString());
            else if (char.IsDigit(expression[i])) // operand
            {
                var operand = "";
                for (; i < expression.Length && char.IsDigit(expression[i]); i++)
                {
                    operand += expression[i];
                }
                i--;
                stack.Push(operand);
            }
            else if (expression[i] == ')') // calc expr
            {
                var operands = new Stack<int>();
                while (stack.Peek().Length > 1 || ! _operators.ContainsKey(stack.Peek()[0]))
                {
                    operands.Push(int.Parse(stack.Pop()));
                }

                if (operands.Count <= 1)
                    throw new InvalidExpressionException("Expected at least two operands.");

                var op = stack.Pop()[0];

                stack.Push(_operators[op](operands.ToList()).ToString());
            }
        }

        return int.Parse(stack.Pop());
    }

    private void ValidateExpression(string expression)
    {
        if (expression.Length == 0)
        {
            throw new InvalidExpressionException("Empty expression.");
        }

        if (expression[0] != '(')
        {
            throw new InvalidExpressionException("Expression has to start with '('.");
        }

        if (expression[^1] != ')')
        {
            throw new InvalidExpressionException("Expression has to end with ')'.");
        }
        
        for (var i = 0; i < expression.Length; i++)
        {
            switch (expression[i])
            {
                case '(':
                    if (i - 1 >= 0 && expression[i - 1] != ' ')
                        throw new InvalidExpressionException("Expected ' ' before '('.");
                    if (i + 1 >= expression.Length || _operators.ContainsKey(expression[i + 1]) == false)
                        throw new InvalidExpressionException("Expected operator after '('.");
                    break;

                case '+':
                case '-':
                case '*':
                case '/':
                    if (i - 1 < 0 || expression[i - 1] != '(')
                        throw new InvalidExpressionException("Expected '(' before operator.");
                    if (i + 1 >= expression.Length || expression[i + 1] != ' ')
                        throw new InvalidExpressionException("Expected ' ' after operator.");
                    break;

                case ' ':
                    if (i - 1 < 0 || (_operators.ContainsKey(expression[i - 1]) != true && char.IsDigit(expression[i - 1]) != true && expression[i - 1] != ')'))
                        throw new InvalidExpressionException("Expected operator or operand before ' '.");
                    if (i + 1 >= expression.Length || (char.IsDigit(expression[i + 1]) != true && expression[i + 1] != '('))
                        throw new InvalidExpressionException("Expected operand after ' '.");
                    break;

                case ')':
                    if (i - 1 < 0 || (char.IsDigit(expression[i - 1]) != true && expression[i - 1] != ')'))
                        throw new InvalidExpressionException("Expected operand before ')'.");
                    if (i + 1 < expression.Length && expression[i + 1] != ' ' && expression[i + 1] != ')')
                        throw new InvalidExpressionException("Expected operand after ' '.");
                    break;

                default:
                    if (i - 1 < 0 || expression[i - 1] != ' ')
                        throw new InvalidExpressionException("Expected ' ' before operand.");

                    var operand = "";
                    for (; i < expression.Length && char.IsDigit(expression[i]); i++)
                    {
                        operand += expression[i];
                    }
                    // compensate for excessive addition in last for loop
                    i--;
                    if (operand.Length <= 0)
                    {
                        throw new InvalidExpressionException("Invalid expression.");
                    }
                    break;
            }
        }
    }

}
