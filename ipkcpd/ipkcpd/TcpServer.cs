using System.Net;
using System.Net.Sockets;

namespace ipkcpd;

public class TcpServer : IServer
{
    private TcpListener? _tcpListener;
    private readonly List<TcpClientHandler> _handlers = new();
    private bool _disposed = false;

    public void Run(string ip, int port)
    {
        try
        {
            var localAddress = IPAddress.Parse(ip);

            // Create a server.
            _tcpListener = new TcpListener(localAddress, port);

            // Start listening for client requests.
            _tcpListener.Start();

            Console.WriteLine($"TCP server started. Listening on {localAddress}:{port}...");

            // Enter the listening loop.
            while (! _disposed)
            {
                Console.WriteLine("Waiting for a connection...");

                // Perform a blocking call to accept requests.
                var client = _tcpListener.AcceptTcpClient();
                if (_disposed) break;
                Console.WriteLine("Connected!");

                // Create a new task to handle the client connection asynchronously.
                var handler = new TcpClientHandler(client);
                _handlers.Add(handler);
                Task.Run(handler.HandleClient);
            }
        }
        catch (SocketException e)
        {
            Console.WriteLine($"SocketException: {e}");
        }
    }



    public void Dispose()
    {
        if (_disposed) return;
        // Stop all the clients
        foreach (var handler in _handlers)
        {
            handler.Dispose();
        }

        // Stop listening for new clients.
        _tcpListener?.Stop();

        _disposed = true;
    }
}