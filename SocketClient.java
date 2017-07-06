package socketClient;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Scanner;
import java.util.Set;

public class SocketClient {
	private static String IP_ADDRESS = "192.168.31.137";
	private static int PORT = 9090;
	private int BUFFERSIZE=1024;
	
	private Selector selector=null;
	private SocketChannel socketChannel=null;
	private HashMap<Object,Object> clientChannelMap=null;
	
	public void initialize(){
		try {
			clientChannelMap=new HashMap<>();
			selector=Selector.open();
			socketChannel=SocketChannel.open();
			socketChannel.configureBlocking(false);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public void finalize() throws IOException{
		socketChannel.close();
		selector.close();
	}
	
	public String decode(ByteBuffer byteBuffer) throws CharacterCodingException{
		Charset charset=Charset.forName("ISO-8859-1");
		CharsetDecoder decoder=charset.newDecoder();
		CharBuffer charBuffer=decoder.decode(byteBuffer) ;
		String result=charBuffer.toString();
		return result;
	}
	
	public void connectServer() throws IOException{
		InetSocketAddress isa=new InetSocketAddress(IP_ADDRESS,PORT );
		socketChannel.connect(isa);
	}
	
	public void portListening() throws IOException{
		InetSocketAddress isa=new InetSocketAddress(IP_ADDRESS,PORT );
		socketChannel.connect(isa);
		
		SelectionKey acceptkey=socketChannel.register(selector, SelectionKey.OP_CONNECT);
		while(acceptkey.selector().select() > 0){
//			System.out.println("event happend");
			Set readyKeys=this.selector.selectedKeys();
			Iterator iterator=readyKeys.iterator();
			while (iterator.hasNext()) {
				SelectionKey key=(SelectionKey) iterator.next();
				iterator.remove();
				if (key.isAcceptable()) {
					System.out.println("more client connect in ");
					
//					ServerSocketChannel nextReady=(ServerSocketChannel) key.channel();
//					SocketChannel sc=nextReady.accept();
//					sc.configureBlocking(false);

				}else if(key.isReadable()){
//					System.out.println("Readable");

//					readFromChannel(socketChannel,clientChannelMap.get(socketChannel.socket()));
//					key.interestOps(SelectionKey.OP_WRITE);
					
				}else if (key.isWritable()) {
//					System.out.println("Writable");
	
//					writeToChannel(socketChannel,"this is a message from server");
//					key.interestOps(SelectionKey.OP_READ);
					
				}else if (key.isConnectable()) {
					System.out.println("Connectable");
					if (socketChannel.finishConnect()) {
						System.out.println("now status is connected ");
						key.interestOps(SelectionKey.OP_WRITE | SelectionKey.OP_READ);
					}
				}
			}
		}
	}
	
	private int writeToChannel(String message) throws IOException {
		SelectionKey acceptkey=socketChannel.register(selector, SelectionKey.OP_CONNECT );
		while(acceptkey.selector().select() > 0){
			Set readyKeys=this.selector.selectedKeys();
			Iterator iterator=readyKeys.iterator();
			while (iterator.hasNext()) {
				SelectionKey key=(SelectionKey) iterator.next();
				iterator.remove();
				if(key.isWritable()){
					System.out.println("write a message to server");
					ByteBuffer buf=ByteBuffer.wrap(message.getBytes());
					int nbytes=socketChannel.write(buf);
					return nbytes;
				}else if (key.isConnectable()) {
					System.out.println("Connectable");
					if (socketChannel.finishConnect()) {
						System.out.println("now status is connected ");
						key.interestOps(SelectionKey.OP_WRITE);
					}
				}
			}
		}
		return 0;
	}
	
	private int writeToChannel(SocketChannel channel, String message) throws IOException {
		ByteBuffer buf=ByteBuffer.wrap(message.getBytes());
		int nbytes=channel.write(buf);
	
		return nbytes;
	}
	
	private String readFromChannel() throws IOException {
		SelectionKey acceptkey=socketChannel.register(selector, SelectionKey.OP_CONNECT);
		while(acceptkey.selector().select() > 0){
			Set readyKeys=this.selector.selectedKeys();
			Iterator iterator=readyKeys.iterator();
			while (iterator.hasNext()) {
				SelectionKey key=(SelectionKey) iterator.next();
				iterator.remove();
				if(key.isReadable()){
					ByteBuffer byteBuffer=ByteBuffer.allocate(BUFFERSIZE);
					int nbytes=socketChannel.read(byteBuffer);
					byteBuffer.flip();
					String result=this.decode(byteBuffer);
					if (result.indexOf("@exit") >= 0) {
						socketChannel.close();
						return null;
					} else {
							System.out.println("receive message from client,"+"Msg : "+result+
									" , IP1 : "+socketChannel.getLocalAddress()+", IP2 : "+socketChannel.getRemoteAddress());
							return result;
						}
					}else if (key.isConnectable()) {
						System.out.println("Connectable");
						if (socketChannel.finishConnect()) {
							System.out.println("now status is connected ");
							key.interestOps(SelectionKey.OP_READ);
						}
				}
			}
		}
		return null;
	}
	
	private void readFromChannel(SocketChannel channel, Object object) throws IOException {
		ByteBuffer byteBuffer=ByteBuffer.allocate(BUFFERSIZE);
		int nbytes=channel.read(byteBuffer);
		byteBuffer.flip();
		String result=this.decode(byteBuffer);
		if (result.indexOf("@exit") >= 0) {
			channel.close();
		}else {
			System.out.println("receive message from client,"+"Msg : "+result+" , IP1 : "+channel.getLocalAddress()+", IP2 : "+channel.getRemoteAddress());
		}
	}
	public class ClientChInstance{
		SocketChannel channel;
		StringBuffer buffer=new StringBuffer();
		public ClientChInstance(SocketChannel channel){
			this.channel=channel;
		}
		public void execute() throws IOException{
			String message="this is response after reading from channel";
			writeToChannel(this.channel, message);
			buffer=new StringBuffer();
		}
		
		public void append(String values){
			buffer.append(values);
		}
	}

	
	
	public static void main(String[] args) throws IOException {
		SocketClient client=new SocketClient();
		client.initialize();
		client.connectServer();
		
//		Thread thread=new Thread(){
//			public void run() {
//				try {
//					client.portListening();
//					
//				} catch (IOException e) {
//					e.printStackTrace();
//				}
//			};
//		};
//		thread.start();
		
		//client.writeToChannel( "this is a message from client ");
		System.out.println("result = "+client.readFromChannel());
		
		Scanner scanner=new Scanner(System.in);
		scanner.nextLine();
	}
}