import torch
import configparser
config = configparser.ConfigParser()
config.read('param.ini')
class KalmanFilter:
    def __init__(self,F,H,B,Q,R):
        self.F=F;self.H=H;self.B=B;self.Q=Q;self.R=R
        self.x=None;self.P=None
    def init(self,x0,P0):
        self.x=x0;self.P=P0
    def predict(self,u):
        self.x=self.F@self.x+self.B@u
        self.P=self.F@self.P@self.F.T+self.Q
    def update(self,z): 
        S=self.H@self.P@self.H.T+self.R
        K=torch.linalg.solve(
           S.T,(self.P@self.H.T).T).T
        self.x=self.x+K@(z-self.H@self.x)
        I=torch.eye(self.P.shape[0],device=self.P.device)
        self.P=(I-K@self.H)@self.P@(I-K@self.H).T+K@self.R@K.T
        return self.x,self.P
    def run_sequence(self, x0, P0, z_seq, u_seq=None):
        T=z_seq.shape[0]
        x_est=torch.zeros((T,x0.shape[0]),device=x0.device)
        if u_seq is None:
            u_seq=torch.zeros((T,self.B.shape[1]),device=x0.device)
        self.init(x0,P0)
        for t in range(T):
            self.predict(u_seq[t])
            self.update(z_seq[t])
            x_est[t]=self.x.reshape(-1)
        return x_est
class Runner:
    def __init__(self,F=None,H=None,B=None,Q=None,R=None):
        self.device=torch.device("cuda" if torch.cuda.is_available() else "cpu")
        if F is None:
            F=torch.tensor(eval(config.get('F','F'))).to(self.device)
        if H is None:
            H=torch.tensor(eval(config.get('H','H'))).to(self.device)
        if B is None:
            B=torch.tensor(eval(config.get('B','B'))).to(self.device)
        if Q is None:
            Q=torch.tensor(eval(config.get('Q','Q'))).to(self.device)
        if R is None:
            R=torch.tensor(eval(config.get('R','R'))).to(self.device)
    def run(self,x_true,z_seq,P0=None,u_seq=None):
        kf=KalmanFilter(self.F,self.H,self.B,self.Q,self.R)
        if P0 is None:
            P0=torch.eye(self.F.shape[0],device=self.F.device)
        if u_seq is None:
            u_seq=torch.zeros((z_seq.shape[0],self.B.shape[1]),device=self.F.device)
        x_est=kf.run_sequence(x_true,P0,z_seq,u_seq)
        return x_est
if __name__=="__main__":
    runner = Runner()