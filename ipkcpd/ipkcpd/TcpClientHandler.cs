using System.Net.Sockets;

namespace ipkcpd;

internal class TcpClientHandler : IDisposable
{
    private readonly TcpClient _client;
    private readonly NetworkStream _stream;
    private readonly StreamReader _reader;
    private readonly StreamWriter _writer;

    private bool _disposed;

    public TcpClientHandler(TcpClient client)
    {
        _client = client;

        // Get a stream object for reading and writing
        _stream = _client.GetStream();

        // Create a reader and writer for the stream.
        _reader = new StreamReader(_stream);
        _writer = new StreamWriter(_stream);
    }


    private void SayResult(TextWriter writer, int result)
    {
        Console.WriteLine($"Result: {result}");
        writer.WriteLine($"RESULT {result}");
        writer.Flush();
    }


    public void HandleClient()
    {
        try
        {
            var message =
                // get HELLO
                _reader.ReadLine();
            if (message != "HELLO")
            {
                Console.WriteLine("Communication did not start with HELLO, ending connection.");
                Dispose();
                return;
            }

            Console.WriteLine("On HELLO answering with HELLO.");
            _writer.WriteLine("HELLO");
            _writer.Flush();

            // Enter the message reading loop.
            while (true)
            {
                message = _reader.ReadLine();
                if (message is null)
                {
                    Console.WriteLine("Message was empty.");
                    Dispose();
                    return;
                }

                Console.WriteLine($"Received: {message}");

                if (!message.StartsWith("SOLVE "))
                {
                    Console.WriteLine("Expression did not start with \"SOLVE \", ending connection.");
                    Dispose();
                    return;
                }

                var expression = message[6..]; // remove "SOLVE "

                // Solve the request
                try
                {
                    var result = new ExpressionSolver().Solve(expression);
                    if (result < 0)
                    {
                        Console.WriteLine("Expression was negative.");
                        Dispose();
                        return;
                    }

                    SayResult(_writer, result);
                }
                catch (InvalidExpressionException e)
                {
                    Console.WriteLine(e.Message);
                    Dispose();
                    return;
                }
            }
        }
        catch (IOException)
        {
            Console.WriteLine("Client must have remotely close the connection.");
        }
    }

    public void Dispose()
    {
        if (_disposed) return;

        // Send last BYE
        Console.WriteLine("Ending with BYE.");
        try
        {
            _writer.WriteLine("BYE");
            _writer.Flush();
        }
        catch (Exception)
        {
            Console.WriteLine("Client must have remotely close the connection.");
        }

        // Dispose of resources
        _writer.Dispose();
        _reader.Dispose();
        _stream.Dispose();
        _client.Dispose();

        _disposed = true;
    }
}
