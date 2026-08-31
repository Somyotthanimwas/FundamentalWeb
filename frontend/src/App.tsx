import { useEffect, useMemo, useState } from 'react'
import './App.css'

type ApiStock = {
  symbol: string
  last: string
  percentChange: string
  volume: string
  value: string
  marketCap: string
  pe: string
  pbv: string
  deRatio: string
  dps: string
  eps: string
  roa: string
  roe: string
  netProfitMargin: string
  dividendYield: string
  bookValuePerShare: string
  listedShare: string
}

type Stock = ApiStock & {
  price: number
  change: number
  peValue: number
  pbvValue: number
  roeValue: number
  roaValue: number
  npmValue: number
  dividendValue: number
}

type SortKey =
  | 'symbol'
  | 'roe'
  | 'roa'
  | 'npm'
  | 'dividend'
  | 'pe'
  | 'pbv'

const PAGE_SIZE = 50

function toNumber(value: string) {
  const number = Number(value)
  return Number.isFinite(number) ? number : 0
}

function normalizeStock(stock: ApiStock): Stock {
  return {
    ...stock,
    price: toNumber(stock.last),
    change: toNumber(stock.percentChange),
    peValue: toNumber(stock.pe),
    pbvValue: toNumber(stock.pbv),
    roeValue: toNumber(stock.roe),
    roaValue: toNumber(stock.roa),
    npmValue: toNumber(stock.netProfitMargin),
    dividendValue: toNumber(stock.dividendYield),
  }
}

function displayNumber(value: string, digits = 2) {
  return value !== '' && Number.isFinite(Number(value))
    ? Number(value).toFixed(digits)
    : '—'
}

function App() {
  const [stocks, setStocks] = useState<Stock[]>([])
  const [search, setSearch] = useState('')
  const [sortBy, setSortBy] = useState<SortKey>('symbol')
  const [sortDirection, setSortDirection] = useState<'asc' | 'desc'>('asc')
  const [page, setPage] = useState(1)
  const [selectedStock, setSelectedStock] = useState<Stock | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')

  useEffect(() => {
    let cancelled = false

    async function loadStocks() {
      try {
        setLoading(true)
        setError('')

        const response = await fetch('https://fundamentalweb.onrender.com/api/stocks')

        if (!response.ok) {
          throw new Error(`HTTP ${response.status}`)
        }

        const data: ApiStock[] = await response.json()

        if (!cancelled) {
          setStocks(data.map(normalizeStock))
        }
      } catch (err) {
        if (!cancelled) {
          setError(
            err instanceof Error
              ? err.message
              : 'Unable to load stock data',
          )
        }
      } finally {
        if (!cancelled) {
          setLoading(false)
        }
      }
    }

    loadStocks()

    const refreshTimer = window.setInterval(() => {
      loadStocks()
    }, 5000)

    return () => {
      cancelled = true
      window.clearInterval(refreshTimer)
    }
  }, [])

  useEffect(() => {
    setPage(1)
  }, [search, sortBy])

  const filteredStocks = useMemo(() => {
    const keyword = search.trim().toLowerCase()

    const result = stocks.filter((stock) =>
      stock.symbol.toLowerCase().includes(keyword),
    )

    result.sort((a, b) => {
      if (sortBy === 'symbol') {
        const difference = a.symbol.localeCompare(b.symbol)
        return sortDirection === 'asc' ? difference : -difference
      }

      const values: Record<
        Exclude<SortKey, 'symbol'>,
        (stock: Stock) => number
      > = {
        roe: (stock) => stock.roeValue,
        roa: (stock) => stock.roaValue,
        npm: (stock) => stock.npmValue,
        dividend: (stock) => stock.dividendValue,
        pe: (stock) => stock.peValue,
        pbv: (stock) => stock.pbvValue,
      }

      const difference = values[sortBy](b) - values[sortBy](a)
      return sortDirection === 'asc' ? difference : -difference
    })

    return result
  }, [stocks, search, sortBy, sortDirection])

  const totalPages = Math.max(
    1,
    Math.ceil(filteredStocks.length / PAGE_SIZE),
  )

  const currentPage = Math.min(page, totalPages)

  const pageStocks = useMemo(() => {
    const start = (currentPage - 1) * PAGE_SIZE
    return filteredStocks.slice(start, start + PAGE_SIZE)
  }, [filteredStocks, currentPage])

  const pageStart =
    filteredStocks.length === 0
      ? 0
      : (currentPage - 1) * PAGE_SIZE + 1

  const pageEnd = Math.min(
    currentPage * PAGE_SIZE,
    filteredStocks.length,
  )

  const averageRoe = useMemo(() => {
    const valid = stocks.filter(
      (stock) => stock.roe.trim() !== '' && Number.isFinite(Number(stock.roe)),
    )

    if (!valid.length) return 0

    return (
      valid.reduce((sum, stock) => sum + stock.roeValue, 0) /
      valid.length
    )
  }, [stocks])

  const visiblePages = useMemo(() => {
    const pages: number[] = []

    const start = Math.max(1, currentPage - 2)
    const end = Math.min(totalPages, currentPage + 2)

    for (let number = start; number <= end; number += 1) {
      pages.push(number)
    }

    return pages
  }, [currentPage, totalPages])

  return (
    <div className="app">
      <header className="topbar">
        <div>
          <div className="brand">FUNDAMENTAL V4</div>
          <div className="subtitle">
            Thai Stock Fundamental Dashboard
          </div>
        </div>

        <div className="header-status">
          <span className="status-dot" />
          {loading ? 'LOADING DATA' : 'DATA ENGINE ONLINE'}
        </div>
      </header>

      <main className="container">
        <section className="hero">
          <div>
            <p className="eyebrow">SET MARKET ANALYTICS</p>
            <h1>Fundamental Dashboard</h1>
            <p className="hero-text">
              วิเคราะห์ราคาและข้อมูลพื้นฐานหุ้นไทยจาก Fundamental V4
            </p>
          </div>

          <div className="search-box">
            <span>⌕</span>
            <input
              type="search"
              placeholder="Search symbol..."
              value={search}
              onChange={(event) => setSearch(event.target.value)}
            />
          </div>
        </section>

        {error && (
          <div className="error-box">
            Backend connection error: {error}
          </div>
        )}

        <section className="stats-grid">
          <article className="stat-card">
            <span>STOCKS</span>
            <strong>{stocks.length.toLocaleString()}</strong>
            <small>Fundamental V4 symbols</small>
          </article>

          <article className="stat-card">
            <span>DATA SOURCE</span>
            <strong className="online">V4 CSV</strong>
            <small>Live backend data</small>
          </article>

          <article className="stat-card">
            <span>AVERAGE ROE</span>
            <strong>{averageRoe.toFixed(2)}%</strong>
            <small>Valid ROE values</small>
          </article>

          <article className="stat-card">
            <span>STATUS</span>
            <strong className={loading ? '' : 'online'}>
              {loading ? 'LOADING' : 'READY'}
            </strong>
            <small>C++ Backend :8080</small>
          </article>
        </section>

        {selectedStock && (
          <section className="section stock-detail">
            <div className="section-heading">
              <div>
                <p className="eyebrow">STOCK DETAIL</p>
                <h2>{selectedStock.symbol}</h2>
              </div>

              <button
                type="button"
                className="detail-close"
                onClick={() => setSelectedStock(null)}
              >
                Close
              </button>
            </div>

            <div className="detail-grid">
              <div className="detail-card">
                <span>PRICE</span>
                <strong>{displayNumber(selectedStock.last)}</strong>
              </div>

              <div className="detail-card">
                <span>CHANGE</span>
                <strong className={
                  selectedStock.change >= 0 ? 'positive' : 'negative'
                }>
                  {selectedStock.percentChange !== ''
                    ? `${selectedStock.change >= 0 ? '+' : ''}${selectedStock.change.toFixed(2)}%`
                    : '—'}
                </strong>
              </div>

              <div className="detail-card">
                <span>P/E</span>
                <strong>{displayNumber(selectedStock.pe)}</strong>
              </div>

              <div className="detail-card">
                <span>P/BV</span>
                <strong>{displayNumber(selectedStock.pbv)}</strong>
              </div>

              <div className="detail-card">
                <span>D/E</span>
                <strong>{displayNumber(selectedStock.deRatio)}</strong>
              </div>

              <div className="detail-card">
                <span>EPS</span>
                <strong>{displayNumber(selectedStock.eps)}</strong>
              </div>

              <div className="detail-card">
                <span>DPS</span>
                <strong>{displayNumber(selectedStock.dps)}</strong>
              </div>

              <div className="detail-card">
                <span>ROE</span>
                <strong>{displayNumber(selectedStock.roe, 1)}%</strong>
              </div>

              <div className="detail-card">
                <span>ROA</span>
                <strong>{displayNumber(selectedStock.roa, 1)}%</strong>
              </div>

              <div className="detail-card">
                <span>NPM</span>
                <strong>{displayNumber(selectedStock.netProfitMargin, 1)}%</strong>
              </div>

              <div className="detail-card">
                <span>DIVIDEND YIELD</span>
                <strong>{displayNumber(selectedStock.dividendYield, 2)}%</strong>
              </div>

              <div className="detail-card">
                <span>BOOK VALUE / SHARE</span>
                <strong>{displayNumber(selectedStock.bookValuePerShare)}</strong>
              </div>

              <div className="detail-card">
                <span>MARKET CAP</span>
                <strong>{displayNumber(selectedStock.marketCap)}</strong>
              </div>

              <div className="detail-card">
                <span>LISTED SHARES</span>
                <strong>{displayNumber(selectedStock.listedShare)}</strong>
              </div>
            </div>
          </section>
        )}

        <section className="section">
          <div className="section-heading">
            <div>
              <p className="eyebrow">STOCK UNIVERSE</p>
              <h2>Thai Stocks</h2>
            </div>

            <div className="results">
              {filteredStocks.length === 0
                ? 'No results'
                : `Showing ${pageStart.toLocaleString()}–${pageEnd.toLocaleString()} of ${filteredStocks.length.toLocaleString()}`}
            </div>
          </div>

          <div className="table-wrap">
            <table>
              <thead>
                <tr>
                  <th>Symbol</th>
                  <th>Price</th>
                  <th>Change</th>
                  <th>P/E</th>
                  <th>P/BV</th>
                  <th>ROE</th>
                  <th>ROA</th>
                  <th>NPM</th>
                  <th>Dividend</th>
                </tr>
              </thead>

              <tbody>
                {pageStocks.map((stock) => (
                  <tr key={stock.symbol}>
                    <td>
                      <button
                        type="button"
                        className="symbol symbol-button"
                        onClick={() => setSelectedStock(stock)}
                      >
                        {stock.symbol}
                      </button>
                    </td>

                    <td className="number price">
                      {displayNumber(stock.last)}
                    </td>

                    <td
                      className={`number ${
                        stock.change >= 0
                          ? 'positive'
                          : 'negative'
                      }`}
                    >
                      {stock.percentChange !== ''
                        ? `${stock.change >= 0 ? '+' : ''}${stock.change.toFixed(2)}%`
                        : '—'}
                    </td>

                    <td className="number">
                      {displayNumber(stock.pe)}
                    </td>

                    <td className="number">
                      {displayNumber(stock.pbv)}
                    </td>

                    <td className="number">
                      {stock.roe !== ''
                        ? `${stock.roeValue.toFixed(1)}%`
                        : '—'}
                    </td>

                    <td className="number">
                      {stock.roa !== ''
                        ? `${stock.roaValue.toFixed(1)}%`
                        : '—'}
                    </td>

                    <td className="number">
                      {stock.netProfitMargin !== ''
                        ? `${stock.npmValue.toFixed(1)}%`
                        : '—'}
                    </td>

                    <td className="number">
                      {stock.dividendYield !== ''
                        ? `${stock.dividendValue.toFixed(2)}%`
                        : '—'}
                    </td>
                  </tr>
                ))}

                {!loading && filteredStocks.length === 0 && (
                  <tr>
                    <td colSpan={9} className="empty">
                      No stocks found.
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>

          {filteredStocks.length > 0 && (
            <div className="pagination">
              <button
                type="button"
                disabled={currentPage === 1}
                onClick={() => setPage((value) => Math.max(1, value - 1))}
              >
                ← Previous
              </button>

              {visiblePages.map((number) => (
                <button
                  type="button"
                  key={number}
                  className={number === currentPage ? 'active' : ''}
                  onClick={() => setPage(number)}
                >
                  {number}
                </button>
              ))}

              <button
                type="button"
                disabled={currentPage === totalPages}
                onClick={() =>
                  setPage((value) => Math.min(totalPages, value + 1))
                }
              >
                Next →
              </button>

              <span className="page-info">
                Page {currentPage} / {totalPages}
              </span>
            </div>
          )}
        </section>

        <section className="section ranking-section">
          <div className="section-heading">
            <div>
              <p className="eyebrow">FUNDAMENTAL SCREENING</p>
              <h2>Sort &amp; Rank</h2>
            </div>
          </div>

          <div className="sort-buttons">
            {[
              ['symbol', 'Symbol'],
              ['roe', 'ROE'],
              ['roa', 'ROA'],
              ['npm', 'NPM'],
              ['dividend', 'Dividend'],
              ['pe', 'P/E'],
              ['pbv', 'P/BV'],
            ].map(([key, label]) => (
              <button
                type="button"
                key={key}
                className={sortBy === key ? 'active' : ''}
                onClick={() => {
                  const nextKey = key as SortKey

                  if (sortBy === nextKey) {
                    setSortDirection((direction) =>
                      direction === 'asc' ? 'desc' : 'asc',
                    )
                  } else {
                    setSortBy(nextKey)
                    setSortDirection('asc')
                  }
                }}
              >
                {label}
                {sortBy === key && (
                  <span className="sort-arrow" aria-hidden="true">
                    {sortDirection === 'asc' ? '↓' : '↑'}
                  </span>
                )}
              </button>
            ))}
          </div>
        </section>
      </main>

      <footer>
        <span>Fundamental V4</span>
        <span>869 SET symbols · C++ Backend</span>
      </footer>
    </div>
  )
}

export default App
