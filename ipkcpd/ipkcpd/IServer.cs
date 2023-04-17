namespace ipkcpd;

public interface IServer : IDisposable
{
    public void Run(string ip, int port);
}