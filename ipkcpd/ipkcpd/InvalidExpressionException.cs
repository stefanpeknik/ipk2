namespace ipkcpd;

public class InvalidExpressionException : Exception
{
    public InvalidExpressionException(string message) : base(message)
    {
    }
}