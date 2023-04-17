using System.Net;
using System.Net.Sockets;
using System.Text;
// ReSharper disable FunctionNeverReturns

namespace ipkcpd;

public class UdpServer : IServer
{
    private UdpClient? _udpServer;
    private bool _disposed = false;

    public void Run(string ip, int port)
    {
        var localEndPoint = new IPEndPoint(IPAddress.Parse(ip), port);
        _udpServer = new UdpClient(localEndPoint);

        Console.WriteLine($"UDP server started. Listening on {localEndPoint}...");

        while (true)
        {
            IPEndPoint? remoteEndPoint = null;
            var datagram = _udpServer.Receive(ref remoteEndPoint);

            // Check that the datagram starts with '\0'
            if (datagram[0] != 0)
            {
                Console.WriteLine("Invalid datagram: first byte is not '\\0'");
                continue;
            }

            // Get the length of the message from the second byte
            var messageLength = datagram[1];

            // Check that the datagram is the correct length
            if (datagram.Length != 2 + messageLength)
            {
                Console.WriteLine($"Invalid datagram: expected {2 + messageLength} bytes, got {datagram.Length} bytes");
                continue;
            }

            // Get the message as a string
            var message = Encoding.ASCII.GetString(datagram, 2, messageLength);

            Console.WriteLine($"Received message: {message} from {remoteEndPoint}");

            // Create the response message buffer
            var responseBuffer = new List<byte>
            {
                1 // First byte is '\1' - response
            };

            // Solve the request
            string outMessage;
            try
            {
                var result = new ExpressionSolver().Solve(message);

                if (result < 0)
                {
                    outMessage = "Result is negative.";
                    responseBuffer.Add(1); // Second byte is '\1' - Error
                }
                else
                {
                    outMessage = result.ToString();
                    responseBuffer.Add(0); // Second byte is '\0' - OK
                }
            }
            catch (InvalidExpressionException e)
            {
                outMessage = e.Message;
                responseBuffer.Add(1); // Second byte is '\1' - Error
            }

            var outMessageLen = outMessage.Length;
            responseBuffer.Add((byte)outMessageLen); // Third byte is length of a message

            // Compute the response message
            var responseMessageBytes = Encoding.ASCII.GetBytes(outMessage);

            // Copy the response message into the buffer starting at the fourth byte position
            responseBuffer.AddRange(responseMessageBytes);

            // Send the response to the client
            _udpServer.Send(responseBuffer.ToArray(), outMessageLen + 3, remoteEndPoint);
        }
    }

    public void Dispose()
    {
        if (_disposed) return;
        _udpServer?.Dispose();
        _disposed = true;
    }
}