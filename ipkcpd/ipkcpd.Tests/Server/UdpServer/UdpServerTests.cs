using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ipkcpd.Tests.Server.UdpServer;

public class UdpServerTests
{
    private const string Ip = "127.0.0.1";
    private const int Port = 2023;
    private readonly IPEndPoint _endpoint = new(IPAddress.Parse(Ip), Port);

    public UdpServerTests()
    {
        // Start the server as a Task
        Task.Run(() =>
        {
            var server = new ipkcpd.UdpServer();
            server.Run(Ip, Port);
        });
        Task.Delay(1000); // Wait for the server to start
    }


    private byte[] ConvertMessageToBytesFormat(string message)
    {
        var messageBytes = Encoding.ASCII.GetBytes(message);
        var length = (byte)messageBytes.Length;
        var messageToSend = new List<byte>
        {
            0,
            length
        };
        messageToSend.AddRange(messageBytes);
        return messageToSend.ToArray();
    }

    private byte[] ConvertExpectedWrongResponseToBytesFormat(string message)
    {
        var messageBytes = Encoding.ASCII.GetBytes(message);
        var length = (byte)messageBytes.Length;
        var messageToSend = new List<byte>
        {
            1,
            1,
            length
        };
        messageToSend.AddRange(messageBytes);
        return messageToSend.ToArray();
    }

    private byte[] ConvertExpectedCorrectResponseToBytesFormat(string message)
    {
        var messageBytes = Encoding.ASCII.GetBytes(message);
        var length = (byte)messageBytes.Length;
        var messageToSend = new List<byte>
        {
            1,
            0,
            length
        };
        messageToSend.AddRange(messageBytes);
        return messageToSend.ToArray();
    }

    [Fact]
    public async Task SendCorrectMessage_Receive0OnSecondIndex()
    {
        // Arrange
       
        var message = "(+ 1 2)";


        // Send the message to the server
        var client = new UdpClient();
        client.Send(ConvertMessageToBytesFormat(message), _endpoint);
            

        // Wait for the server to receive and respond to the message
        await Task.Delay(100);

        // Check that the server received and responded with the correct message in the correct format
        var expectedResponse = ConvertExpectedCorrectResponseToBytesFormat("3");

        var response = await client.ReceiveAsync();
        var length = response.Buffer[2] + 3;
        var received = new byte[length];
        Array.Copy(response.Buffer, 0, received, 0, length);

        Assert.Equal(expectedResponse, received);
    }

    [Fact]
    public async Task SendInvalidMessage_Receive1OnSecondIndex()
    {
        // Arrange

        var message = "monke";


        // Send the message to the server
        var client = new UdpClient();
        client.Send(ConvertMessageToBytesFormat(message), _endpoint);


        // Wait for the server to receive and respond to the message
        await Task.Delay(100);

        // Check that the server received and responded with the correct message in the correct format
        var expectedResponse = ConvertExpectedWrongResponseToBytesFormat("3");

        var response = await client.ReceiveAsync();
        var length = response.Buffer[2] + 3;
        var received = new byte[length];
        Array.Copy(response.Buffer, 0, received, 0, length);

        Assert.Equal(expectedResponse[1], received[1]);
    }
}
