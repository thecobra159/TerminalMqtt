import ReactTable from 'react -table'
import * as React from 'react'

export default class Transactions extends React.Component<Props, {}> {
    public columns = [
      {
        Header: 'Id',
        accessor: 'employersId',
        minWidth: 50,
      },
      {
        Header: 'Entrada',
        acessor: 'employersCheckIn',
        minWidth: 50,
      },
      {
        Header: 'Saída',
        acessor: 'employersCheckOut',
        minWidth: 50,
      }
    ]

    public render(): React.ReactNode {
        return (
          <div id='table-employers'>
            <div>
          </div>
        )
    }
}