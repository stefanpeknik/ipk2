using System.Net.Sockets;

namespace ipkcpd.Tests.Server.TcpServer;

public class TcpServerTests : IDisposable
{
    private const string Ip = "127.0.0.1";
    private const int Port = 2023;
    private readonly ipkcpd.TcpServer _server;
    private readonly Task _serverTask;

    public TcpServerTests()
    {
        // Start the server as a Task
        _server = new ipkcpd.TcpServer();
        _serverTask = Task.Run(() =>
        {
            _server.Run(Ip, Port);
        });
    }


    [Fact]
    public Task DisconnectClientWithoutBye_NotThrow()
    {
        // Arrange
        using var client = new TcpClient(Ip, Port);
        using var writer = new StreamWriter(client.GetStream());
        using var reader = new StreamReader(client.GetStream());

        // Act
        writer.WriteLine("HELLO");
        writer.Flush();
        client.Close();
        return Task.CompletedTask;
    }

    [Fact]
    public async Task SendFirstHello_GetHello()
    {
        // Arrange
        using var client = new TcpClient(Ip, Port);
        using var writer = new StreamWriter(client.GetStream());
        using var reader = new StreamReader(client.GetStream());
        await Task.Delay(1000);

        // Act
        writer.WriteLine("HELLO");
        writer.Flush();
        await Task.Delay(1000);

        // Assert
        var result = reader.ReadLine();
        Assert.Equal("HELLO", result);
    }

    [Fact]
    public async Task SendInvalidMessage_GetBye()
    {
        // Arrange
        using var client = new TcpClient(Ip, Port);
        using var writer = new StreamWriter(client.GetStream());
        using var reader = new StreamReader(client.GetStream());
     
        // Act
        writer.WriteLine("monke");
        writer.Flush();

        await Task.Delay(1000);
        
        // Assert
        var result = reader.ReadLine();
        Assert.Equal("BYE", result);
    }

    [Fact]
    public Task MessageWithTwoLines_CorrectResults()
    {
        // Arrange
        using var client = new TcpClient(Ip, Port);
        using var writer = new StreamWriter(client.GetStream());
        using var reader = new StreamReader(client.GetStream());

        // Act
        writer.WriteLine("HELLO");
        writer.Flush();
        writer.WriteLine("SOLVE (+ 1 1)\nSOLVE (+ 2 2)");
        writer.Flush();

        // Assert
        var resultHello = reader.ReadLine();
        Assert.Equal("HELLO", resultHello);
        var result1 = reader.ReadLine();
        Assert.Equal("RESULT 2", result1);
        var result2 = reader.ReadLine();
        Assert.Equal("RESULT 4", result2);
        return Task.CompletedTask;
    }

    [Fact]
    public Task TwoCorrectExpressionsInARow_CorrectResults()
    {
        // Arrange

        // Connect to the server
        using var client = new TcpClient(Ip, Port);
        using var writer = new StreamWriter(client.GetStream());
        using var reader = new StreamReader(client.GetStream());


        // Act

        // send HELLO
        writer.WriteLine("HELLO");
        writer.Flush();

        // Send a message
        writer.WriteLine("SOLVE (+ 1 1)");
        writer.Flush();

        // Send another message
        writer.WriteLine("SOLVE (- 2 1)");
        writer.Flush();

        // Send a BYE message
        writer.WriteLine("BYE");
        writer.Flush();


        // Assert

        // Receive HELLO
        var resultHello = reader.ReadLine();
        Assert.Equal("HELLO", resultHello);

        // Receive the result
        var result1 = reader.ReadLine();
        Assert.Equal("RESULT 2", result1);

        // Receive another result
        var result2 = reader.ReadLine();
        Assert.Equal("RESULT 1", result2);

        // Receive "BYE"
        var resultBye = reader.ReadLine();
        Assert.Equal("BYE", resultBye);
        return Task.CompletedTask;
    }

    [Fact]
    public Task MultipleClients_NotThrowOrDisconnect()
    {
        // Arrange

        List<TcpClient> clients = new();


        // Act & Assert

        for (var i = 0; i < 10; i++)
        {
            clients.Add(new TcpClient(Ip, Port));
        }

        List<Task> tasks = new();
        foreach (var client in clients)
        {
            tasks.Add(Task.Run(() =>
            {
                using var stream = client.GetStream();
                using var writer = new StreamWriter(stream);
                using var reader = new StreamReader(stream);

                writer.WriteLine("HELLO");
                writer.Flush();

                var response = reader.ReadLine();
                Assert.Equal("HELLO", response);

                writer.WriteLine("BYE");
                writer.Flush();

                response = reader.ReadLine();
                Assert.Equal("BYE", response);
            }));
        }

        Task.WaitAll(tasks.ToArray());
        return Task.CompletedTask;
    }


    public void Dispose()
    {
        _server.Dispose();
    }
}
